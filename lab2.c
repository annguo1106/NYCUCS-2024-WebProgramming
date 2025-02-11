#include "unp.h"
#include <string.h>

void xchg_data(int sockfd){
	
	  char sendline[MAXLINE], recvline[MAXLINE];
	  
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
		
		// initial FD and count
    int count = 0;  // count the number of lines
    int maxfdp1;
    fd_set rset;
    FD_ZERO(&rset);
		while(1){
				// set sockets we want to test
        FD_SET(fileno(stdin), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(stdin), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        // read from stdin
        if(FD_ISSET(fileno(stdin), &rset)){
				    memset(sendline, 0, MAXLINE);
					  // get user input
					  if (Fgets(sendline, MAXLINE, stdin) == NULL) return;
				    count ++;
				    // send this line to server
				    Writen(sockfd, sendline, strlen(sendline));
        }

        // read number from server
        if(FD_ISSET(sockfd, &rset)){
            memset(recvline, 0, MAXLINE);
				    if (Read(sockfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
                
				    // Fputs received line
				    strcat(recvline, "\n");
            Fputs(recvline, stdout);
				    
				    // handle different msg
				    // recv bad -> line doens't count
				    if (strcmp(recvline, "bad") == 0) count --;
				    // recv stop -> send check line to server
	          else if(strcmp(recvline, "stop") == 0){
								memset(sendline, 0, MAXLINE);
                char str_count[MAXLINE];
                // convert (int)count to str
                sprintf(str_count, "%d", count);
								strcat(sendline, str_count);
								strcat(sendline, " ");
                strcat(sendline, ip);
                strcat(sendline, "\n");
								Writen(sockfd, sendline, strlen(sendline));
            }
            // recv ok -> end
				    else if(strcmp(recvline, "ok") == 0) break;
            // recv nak -> error occur, server end connection
            else if(strcmp(recvline, "nak")==0) break;
				}
		}
}


int main(int argc, char **argv){
	int					sockfd;
	struct sockaddr_in	servaddr;
	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");
        
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT + 1);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	// str_cli(stdin, sockfd);		/* do it all */
	Fputs("in xchg data\n", stdout);
	xchg_data(sockfd);

	exit(0);
}

