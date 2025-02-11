#include "unp.h"
#include <string.h>

void tcp_cli(int sockfd){
	
	char sendline[MAXLINE], recvline[MAXLINE], sendline2[MAXLINE];
	
		// get local ip addr using getsockname
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    bzero(&localaddr, sizeof(localaddr));
    getsockname(sockfd, (SA*)&localaddr, &addrlen);
    
    // prepare sendline (student_id + ip)
    char *ip = inet_ntoa(localaddr.sin_addr); // convert sin_addr to str
    char* id = "111550139 ";
    strcat(sendline, id);
    strcat(sendline, ip);
    strcat(sendline, "\n");
    Writen(sockfd, sendline, strlen(sendline));
		Fputs(sendline, stdout);
		while(1){
			// read host name from server
			memset(recvline, 0, MAXLINE);
			if (Read(sockfd, recvline, MAXLINE) == 0)
				err_quit("str_cli: server terminated prematurely");
			printf("get host name: %s\n", recvline);
			// get ip from recved host name
      struct hostent *hostptr;
      if((hostptr = gethostbyname(recvline)) == NULL){
          err_msg("gethostbyname error for host: %s: %s", recvline, hstrerror(h_errno));
          continue;
      }
      char **pptr = hostptr->h_addr_list;
      memset(sendline2, 0, MAXLINE);
      // convert the ip to string
      inet_ntop(hostptr->h_addrtype, *pptr, sendline2, sizeof(sendline2));
      strcat(sendline2, "\n");
      Fputs(sendline2, stdout);
      // send up to serv
			Writen(sockfd, sendline2, strlen(sendline2));
					
			// read from server
			memset(recvline, 0, MAXLINE);
			if (Read(sockfd, recvline, MAXLINE) == 0)
				err_quit("str_cli: server terminated prematurely");
			// print the received msg
			printf("recv from serv: %s\n", recvline);
			// if good -> send student number and ip again
			if (strcmp(recvline, "good") == 0){
            Writen(sockfd, sendline, strlen(sendline));
            memset(recvline, 0, MAXLINE);
            if (Read(sockfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            if (strcmp(recvline, "ok") == 0){
                Fputs("ok\n", stdout);
                break;
            }
            else if (strcmp(recvline, "nak") == 0){
                Fputs("nak\n", stdout);
                break;
          }
      }
      // if bad -> end of connection
			else if (strcmp(recvline, "bad")){
          Fputs("bad\n", stdout);
          break;
      }
	 }
}

int main(int argc, char **argv){
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");
	
	// setup connection
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+2);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	// do things in xchg
	tcp_cli(sockfd);

	exit(0);
}
