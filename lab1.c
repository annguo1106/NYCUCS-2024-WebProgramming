#include "unp.h"
#include <string.h>

int gcd(int a, int b){
	if(b == 0) return a;
	return gcd(b, a%b);
}

void xchg_data(int sockfd){
	
	char sendline[MAXLINE], recvline[MAXLINE];
	
	// let sendline = student id
	char* id = "111550139\n";
	strcpy(sendline, id);
	// write to socket
	Writen(sockfd, sendline, strlen(sendline));
	
	while(1){
		// read two numbers from server
		memset(recvline, 0, MAXLINE);
		if (Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("str_cli: server terminated prematurely");
		
		// convert string to two numbers
		int num1, num2;
		sscanf(recvline, "%d %d", &num1, &num2);
		
		// calculate gcd and convert number to string
		// remember to add "\n" at the end of the string,
		// or Readline() won't treat it as a str
		int answer = gcd(num1, num2);
		memset(sendline, 0, MAXLINE);
		sprintf(sendline, "%d", answer);
		strcat(sendline, "\n");
		// write the answer to socket
		Writen(sockfd, sendline, strlen(sendline));
		
		// read from server
		memset(recvline, 0, MAXLINE);
		if (Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("str_cli: server terminated prematurely");
		// print the received msg
		Fputs(recvline, stdout);
		// if ok -> end
		if (strcmp(recvline, "ok\n") == 0) break;
		// if nak -> continue
		else if (strcmp(recvline, "nak\n")) continue;
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
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
	// do things in xchg
	xchg_data(sockfd);

	exit(0);
}
