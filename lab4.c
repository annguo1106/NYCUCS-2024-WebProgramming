#include "unp.h"
#include <string.h>

// handle concurrent TCP server signal 
void sig_chld(int signo) {
    int stat;
    wait(&stat);
    return;
}

void cli_serv(int sockfd){
	char sendline[MAXLINE], recvline[MAXLINE];
    int listenfd;
    // prepare TCP serv
    struct sockaddr_in TCPserv;
    bzero(&TCPserv, sizeof(TCPserv));
    TCPserv.sin_family = AF_INET;
    TCPserv.sin_addr.s_addr = htonl(INADDR_ANY);
    TCPserv.sin_port = htons(SERV_PORT+3);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(listenfd, (SA *)&TCPserv, sizeof(TCPserv));
    Listen(listenfd, LISTENQ);

    // get self TCP serv ip
    struct sockaddr_in TCPaddr;
    socklen_t TCPaddrlen = sizeof(TCPaddr);
    bzero(&TCPaddr, sizeof(TCPaddr));
    getsockname(sockfd, (SA*)&TCPaddr, &TCPaddrlen);
		
		// send request to teacher's UDP server
    char *ip = inet_ntoa(TCPaddr.sin_addr); // convert sin_addr to str
    snprintf(sendline, sizeof(sendline), "%s %s %s", "11", "111550139", ip);
    Write(sockfd, sendline, strlen(sendline));

    // read host name from server
    memset(recvline, 0, MAXLINE);
    if (Read(sockfd, recvline, MAXLINE) == 0)
        err_quit("str_cli: server terminated prematurely");
    int n;
    char *tmp1 = NULL;
    char *tmp2 = NULL;
    sscanf(recvline, "%d %s %s", &n, tmp1, tmp2);
    if (n >= 90) {  // error occure
        printf("error, n = %d\n", n);
    }
    else {
        // send "13 stu_id TCP_server_port" to UDP server
        memset(sendline, 0, MAXLINE);
        snprintf(sendline, sizeof(sendline), "%s %s %u\n", "13", "111550139", ntohs(TCPserv.sin_port));
		    Write(sockfd, sendline, strlen(sendline));
    }
   
    // read from TCP cli
    socklen_t clilen, peerlen;
    // prepare concurent TCP server
    struct sockaddr_in cliaddr, peeraddr;
    int connfd;
    pid_t childpid;
    Signal(SIGCHLD, sig_chld);
    for(int i = 0; i < n; i++) {
        bzero(&cliaddr, sizeof(cliaddr));
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) < 0) {
            if (errno == EINTR){
                continue;
            }
            else err_sys("accept error");
        }
        if ((childpid = fork()) == 0) {
            Close(listenfd);
            memset(recvline, 0, MAXLINE);
            if (Readline(connfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            memset(sendline, 0, MAXLINE);
            
            // get peer addr
            bzero(&peeraddr, sizeof(peeraddr));
            peerlen = sizeof(peeraddr);
            getpeername(connfd, (SA *)&peeraddr, &peerlen);
	           // format send string (peer addr + received str)
            snprintf(sendline, sizeof(sendline), "%u ", ntohs(peeraddr.sin_port));
            strcat(sendline, recvline);
            Writen(connfd, sendline, strlen(sendline));
            memset(recvline, 0, MAXLINE);
            // read from TCP cli
            if (Readline(connfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            if (strcmp(recvline, "ok")) {
                printf("cli right\n");
            }
            else {
                printf("cli not ok\n");
                printf("%s", recvline);
            }
            exit(0);
        }
        Close(connfd);
    }
    // read from UDP server
    memset(recvline, 0, MAXLINE);
    if (Read(sockfd, recvline, MAXLINE) == 0)
        err_quit("str_cli: server terminated prematurely");
    if (strcmp(recvline, "ok") == 0) {
        printf("udp right, success!!!!!\n");
        return;
    }
    else if (strcmp(recvline, "nak\n") == 0) {
        printf("udp wrong, break\n");
        return;
    }
}

int main(int argc, char **argv){
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");
	
	// setup connection
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+3);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
  // bind UDP socket
  Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    
	// do things in xchg
	cli_serv(sockfd);

	exit(0);
}
