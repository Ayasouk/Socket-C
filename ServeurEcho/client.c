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

int echo(int sockdesc, char * chaine); /* envoi un echo avec comme msg chaine*/
int recv_echo(int sockdesc); /* permet de recevoir la reponse du echo */

int main(int argc, char ** argv){
	int sd, rc;
	double elapse;
	struct sockaddr_in localAddr, servAddr;
	struct hostent *h;
	
	/* variable lié au temps */
	struct timeval time_send; 
	struct timeval time_recv; 	

	if(argc<3){
		printf("usage: %s <server> <data1> <data2>... <dataN>\n",argv[0]);
		exit (1);
	}
	
	h = gethostbyname(argv[1]);
	
	if(h == NULL){
		printf("%s: unknow host '%s'\n",argv[0],argv[1]);
		exit(1);
	}
	
	servAddr.sin_family = h->h_addrtype;
	memcpy((char*) &servAddr.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
	servAddr.sin_port = htons(SERVER_PORT);
	
	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM,0);
	if(sd<0){
		perror("cannot open socket");
		exit(1);
	}
	
	/* bind any port number */
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(0);
	
	rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
	if(rc<0){
		printf("%s: cannot bind port TCP %u\n",argv[0],SERVER_PORT);
		perror("error");
		exit(1);
	}
	
	/* connect to server */
	rc = connect(sd,(struct sockaddr *) &servAddr, sizeof(servAddr)); 
	if(rc<0){
		perror("error");
		exit(1);
	}
	
	/* send the message to echo */
	gettimeofday(&time_send, NULL); /* mesure de la date initial */

	echo(sd, argv[2]);
	recv_echo(sd);
	gettimeofday(&time_recv, NULL); /* mesure de la date apres reponse */
	elapse = (time_recv.tv_usec - time_send.tv_usec);
	printf(" time to answer : %f ms\n", elapse/1000); /* temps de reponse */
	
	close(sd);
	return 0;
}

int echo(int sockdesc, char * chaine){ /* envoi une chaine au server */
	char line[MAX_MSG];
	int n;

	sprintf(line, "%s", chaine);
	n = send(sockdesc, line, MAX_MSG, 0);

	if(n==0) {
		perror("connection fermé\n");
		exit(-1);
	}

	if(n<0) {
		perror("erreur d'envoi de données\n");
		exit(-2);
	}
	return 1;
}

int recv_echo(int sockdesc){
	char line[MAX_MSG];	
	int n;
	n = recv(sockdesc, line, MAX_MSG, 0);
	if(n==0) {
		perror("connection closed\n");
		exit(-1);	
	}
	if(n<0){
		perror("error receiving data\n");
		exit(-2);
	}
	fprintf(stdout, "server : %s\n", line);
	return 1;
}

