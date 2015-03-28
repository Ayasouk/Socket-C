#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> 


#define MAX_MSG 100
#define SERVER_PORT 1500

#define ERROR -1
#define SUCCESS 0


int reply_echo(int sockdesc);

int main (int argc, char * argv[])
{	
	if(argc > 1){
		fprintf(stdout, "erreur, le programme \"%s\" ,e prends pas d'arguments", argv[0]);
		return ERROR;	
	}
	int sd, newSd; 
	socklen_t cliLen ;


	struct sockaddr_in cliAddr, servAddr ;
	
	/*create socket*/
	sd = socket(AF_INET, SOCK_STREAM, 0) ;
	if(sd < 0)
	{
		perror("Cannot open socket !") ;
		return ERROR ;
	}
	
	/* to avoid the error of bind : address already taken*/
	 int optval = 1;
 	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) { 
    		perror("setsockopt");
    		exit(EXIT_FAILURE);
  	}

	/*bind server port*/
	servAddr.sin_family = AF_INET ;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
	servAddr.sin_port = htons(SERVER_PORT) ;
	
	if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		perror("Cannot open socket !") ;
		return ERROR ;
	}
	
	listen(sd, 5) ;
	
	while(1)
	{
		printf("%s : waiting for data on port TCP %u\n", argv[0], SERVER_PORT) ;
		cliLen = sizeof(cliAddr) ;
		newSd = accept(sd, (struct sockaddr *) &cliAddr, &cliLen) ;
		printf("cliaddr.sin_family : %d\n", cliAddr.sin_family);
		printf("cliaddr.sin_addr.s_addr : %s\n", inet_ntoa(cliAddr.sin_addr));
		printf("cliaddr.sin_port : %d\n", cliAddr.sin_port);
		if(newSd < 0)
		{
			perror("Cannot accept connection !") ;
			return ERROR ;
		}
		
		reply_echo(newSd);
	}

	close(sd);
	close(newSd);
}

int reply_echo(int sockdesc){
	char line[MAX_MSG];
	int n;
	n = recv(sockdesc, line, MAX_MSG-1, 0); /* reception du msg */
	if(n==0) { 
		perror("connection closed\n");
		exit(-1);	
	}
	if(n<0){
		perror("error receiving data\n");
		exit(-2);
	}
	line[n] = '\0';

	n = send(sockdesc, line, MAX_MSG, 0); /* envoi du meme msg */
	if(n==0) {
		perror("connection closed\n");
		exit(-1);	
	}
	if(n<0){
		perror("error receiving data\n");
		exit(-2);
	}
	
	return 1;
}

