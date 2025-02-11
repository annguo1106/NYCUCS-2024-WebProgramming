#include "unp.h"
#include <string.h>

void sig_chld(int signo) {
    int stat;
    wait(&stat);
    return;
}

void chat_serv(){
	char sendline[MAXLINE], recvline[MAXLINE];
    int listenfd;

    // prepare TCP serv
    struct sockaddr_in TCPserv;
    bzero(&TCPserv, sizeof(TCPserv));
    TCPserv.sin_family = AF_INET;
    TCPserv.sin_addr.s_addr = htonl(INADDR_ANY);
    TCPserv.sin_port = htons(SERV_PORT+4);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(listenfd, (SA *)&TCPserv, sizeof(TCPserv));
    Listen(listenfd, LISTENQ);

    // set up two client vars
    int cliCount = 0;
    int connfd1, connfd2;
    struct sockaddr_in cliaddr1, cliaddr2;
    socklen_t clilen1, clilen2;
    pid_t childpid;
    char cli1[MAXLINE], cli2[MAXLINE];
    // select
    fd_set rset;
    int maxfdp;
    // sig child
    Signal(SIGCHLD, sig_chld);
    // main
    while(1) {
        if (cliCount == 0) {
            cliCount = 1;
            clilen1 = sizeof(cliaddr1);
            if ((connfd1 = accept(listenfd, (SA *)&cliaddr1, &clilen1)) < 0) {
                if (errno == EINTR){
                    continue;
                }
                else err_sys("accept error");
            }
            // get cli1 name
            memset(recvline, 0, MAXLINE);
            if (Read(connfd1, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            memset(cli1, 0, MAXLINE);
            strcat(cli1, recvline);
            // Writen to first cli
            memset(sendline, 0, MAXLINE);
            strcat(sendline, "You are the 1st user. Wait for the second one!\n");
            Writen(connfd1, sendline, strlen(sendline));
        }
        else if (cliCount == 1) {
            cliCount = 2;
            clilen2 = sizeof(cliaddr2);
            if ((connfd2 = accept(listenfd, (SA *)&cliaddr2, &clilen2)) < 0) {
                if (errno == EINTR){
                    continue;
                }
                else err_sys("accept error");
            }
            // get cli2 name
            memset(recvline, 0, MAXLINE);
            if (Read(connfd2, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            memset(cli2, 0, MAXLINE);
            strcat(cli2, recvline);
            // send to cli2
            memset(sendline, 0, MAXLINE);
            strcat(sendline, "You are the 2nd user\n");
            Writen(connfd2, sendline, strlen(sendline));
        }
        if (cliCount == 2) {  // make pair! fork a chat room for them
            cliCount = 0;
            // send to cli1
            printf("clicount = 2\n");
            memset(sendline, 0, MAXLINE);
            snprintf(sendline, sizeof(sendline), "The second user is %s from %s\n", cli2, inet_ntoa(cliaddr2.sin_addr));
            printf("send %s to cli1\n", sendline);
            Writen(connfd1, sendline, strlen(sendline));
            // send to cli2
            memset(sendline, 0, MAXLINE);
            snprintf(sendline, sizeof(sendline), "The first user is %s from %s\n", cli1, inet_ntoa(cliaddr1.sin_addr));
            printf("send %s to cli2\n", sendline);
            Writen(connfd2, sendline, strlen(sendline));
            FD_ZERO(&rset);
            int stay1 = 1, stay2 = 1;
            if ((childpid = fork()) == 0) {
                Close(listenfd);
                while (1) {
                    FD_ZERO(&rset);
                    if (stay1) FD_SET(connfd1, &rset);
                    if (stay2) FD_SET(connfd2, &rset);
                    maxfdp = max(connfd1, connfd2) + 1;
                    Select(maxfdp, &rset, NULL, NULL, NULL);
                    if (stay1 && FD_ISSET(connfd1, &rset)) {  // recv msg from u1
                        memset(recvline, 0, MAXLINE);
                        if (Read(connfd1, recvline, MAXLINE) == 0) {
                            printf("cli1 leave\n");
                            stay1 = 0;
                            if (!stay2) {  // both leave the chat room
                                memset(sendline, 0, MAXLINE);
                                snprintf(sendline, sizeof(sendline), "(%s left the room)\n", cli1);
                                Writen(connfd2, sendline, strlen(sendline));
                                Shutdown(connfd2, SHUT_WR);
                                break;
                            }
                            else {  // tell cli2 that cli1 has left
                                memset(sendline, 0, MAXLINE);
                                snprintf(sendline, sizeof(sendline), "(%s left the room. Press Ctrl+D to leave.)\n", cli1);
                                Writen(connfd2, sendline, strlen(sendline));
                                Shutdown(connfd2, SHUT_WR);
                            }
                        }
                        else {
                            printf("recv from cli1: %s\n", recvline);
                            memset(sendline, 0, MAXLINE);
                            snprintf(sendline, sizeof(sendline), "(%s) %s", cli1, recvline);
                            Writen(connfd2, sendline, strlen(sendline));
                        }
                    }
                    if(stay2 && FD_ISSET(connfd2, &rset)) {  // recv msg from u2
                        memset(recvline, 0, MAXLINE);
                        if (Read(connfd2, recvline, MAXLINE) == 0) {
                            printf("cli2 leave");
                            stay2 = 0;
                            if (!stay1) {  // both leave the chat room
                                memset(sendline, 0, MAXLINE);
                                snprintf(sendline, sizeof(sendline), "(%s left the room)\n", cli2);
                                Writen(connfd1, sendline, strlen(sendline));
                                Shutdown(connfd1, SHUT_WR);
                                break;
                            }
                            else {  // tell cli1 that cli2 has left
                                memset(sendline, 0, MAXLINE);
                                snprintf(sendline, sizeof(sendline), "(%s left the room. Press Ctrl+D to leave.)\n", cli2);
                                Writen(connfd1, sendline, strlen(sendline));
                                Shutdown(connfd1, SHUT_WR);
                            }
                        }
                        else {
                            printf("recv from cli2: %s\n", recvline);
                            memset(sendline, 0, MAXLINE);
                            snprintf(sendline, sizeof(sendline), "(%s) %s", cli2, recvline);
                            Writen(connfd1, sendline, strlen(sendline));
                        }
                    }
                }
                exit(0);
            }
            Close(connfd1);
            Close(connfd2);
        }
    }
    
}

int main(){
	
    chat_serv();

	exit(0);
}
