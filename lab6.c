#include "unp.h"
#include <string.h>


typedef struct Client{
    int connfd;
    int occupied;
    struct sockaddr_in address;
    socklen_t addrLen;
    char name[MAXLINE];
}Client;

Client clients[11] = {0};

void init_clients () {
    // init clients array
    Client cli; cli.occupied = 0; cli.addrLen = sizeof(cli.address);
    for (int i = 0; i < 10; i++) {
        clients[i] = cli;
        // setsockopt(clients[i].connfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    }
}

void broadcast (int sender, char *msg) {
    printf("broadcast\n");
    for (int i = 0; i < 10; i++) {
        if (clients[i].occupied && i != sender) {
            Writen(clients[i].connfd, msg, strlen(msg));
        }
    }  
}

void chat_serv () {
    char sendline[MAXLINE], recvline[MAXLINE];
    int listenfd;

    // prepare TCP serv
    struct sockaddr_in TCPserv;
    bzero(&TCPserv, sizeof(TCPserv));
    TCPserv.sin_family = AF_INET;
    TCPserv.sin_addr.s_addr = htonl(INADDR_ANY);
    TCPserv.sin_port = htons(SERV_PORT+5);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(listenfd, (SA *)&TCPserv, sizeof(TCPserv));
    Listen(listenfd, LISTENQ);

    int cliCount = 0;
    // select
    fd_set rset;
    int maxfdp;
    FD_ZERO(&rset);
    // main
    while(1) {
        FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        maxfdp = listenfd;
        for (int i = 0; i < 10; i++) {
            if (clients[i].occupied) {
                // printf("set client %s\n", clients[i].name);
                FD_SET(clients[i].connfd, &rset);
                if (clients[i].connfd > maxfdp)  maxfdp = clients[i].connfd;
            } 
        }
        maxfdp ++;
        Select(maxfdp, &rset, NULL, NULL, NULL);
        // if someone sent
        for (int i = 0; i < 10; i++) {
            if (clients[i].occupied && FD_ISSET(clients[i].connfd, &rset)) {
                memset(recvline, 0, MAXLINE);
                if (Read(clients[i].connfd, recvline, MAXLINE) == 0) {
                    printf("read n = 0\n");
                    clients[i].occupied = 0;
                    cliCount --;
                    printf("cli %d leave\n", i);
                    memset(sendline, 0, MAXLINE);
                    snprintf(sendline, sizeof(sendline), "Bye!\n");
                    Writen(clients[i].connfd, sendline, strlen(sendline));
                    Close(clients[i].connfd);
                    memset(sendline, 0, MAXLINE);
                    if (cliCount > 1)
                        snprintf(sendline, sizeof(sendline), "(%s left the room. %d users left)\n", clients[i].name, cliCount);
                    else 
                        snprintf(sendline, sizeof(sendline), "(%s left the room. You are the last one. Press Ctrl+D to leave or wait for a new user.)\n", clients[i].name);
                    // broadcast
                    broadcast(i, sendline);
                }
                // unfinished
                else {
                    printf("recv from cli1: %s\n", recvline);
                    memset(sendline, 0, MAXLINE);
                    snprintf(sendline, sizeof(sendline), "(%s) %s", clients[i].name, recvline);
                    // broadcast
                    broadcast(i, sendline);                    
                }
            }
        }
        if (FD_ISSET(listenfd, &rset)) {
            if (cliCount < 10) {
                cliCount ++;
                int space = 0;
                for (int i = 0; i < 10 ; i++) {
                    if (!clients[i].occupied) {
                        space = i;
                        clients[i].occupied = 1;
                        break;
                    }
                }
                if ((clients[space].connfd = accept(listenfd, (SA *)&clients[space].address, &clients[space].addrLen)) < 0) {
                    if (errno == EINTR){
                        continue;
                    }
                    else err_sys("accept error");
                }
                // get cli name
                memset(recvline, 0, MAXLINE);
                if (Read(clients[space].connfd, recvline, MAXLINE) == 0)
                    err_quit("str_cli: server terminated prematurely");
                memset(clients[space].name, 0, MAXLINE);
                strcat(clients[space].name, recvline);
                // Writen to entered cli
                memset(sendline, 0, MAXLINE);
                snprintf(sendline, sizeof(sendline), "You are the #%d user.\n", cliCount);
                Writen(clients[space].connfd, sendline, strlen(sendline));
                memset(sendline, 0, MAXLINE);
                snprintf(sendline, sizeof(sendline), "You may now type in or wait for other users.\n");
                Writen(clients[space].connfd, sendline, strlen(sendline));
                printf("need broadcast\n");
                // broadcast to the existed clients that a new person entered
                memset(sendline, 0, MAXLINE);
                snprintf(sendline, sizeof(sendline), "(#%d user %s enters)\n", cliCount, clients[space].name);
                printf("user join, print %s\n", sendline);
                broadcast(space, sendline);
            }
        }
    }
    
}

int main(){

    init_clients();
	
    chat_serv();

	exit(0);
}
