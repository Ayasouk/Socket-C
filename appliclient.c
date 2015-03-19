#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define SERVER_PORT 1500
#define MAX_MSG 100

/* definition of the type file request */
typedef struct file_req_s {
   char name[20];
   int size;
   char zero[8]; /* pour que la requete fasse 32bits */
} file_req_t ;

int main(int argc, char ** argv){
	int sd, rc, i;
	struct sockaddr_in localAddr, servAddr;
	struct hostent *h;
	
	/* data linked with the file */
	file_req_t file_req1;
	FILE * fic = NULL;	
	char file_line[MAX_MSG];
	int nb_lu;	

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
	
	/* initialisation of the file request */
	strcpy(file_req1.name, argv[2]);
	file_req1.size = 3;	
	
 	/* send of a file request */
	rc = send(sd, &file_req1, sizeof(file_req_t), 0);
	printf(" send a file request to %s\n", argv[1]);
	printf("file_req.name : %s\n file_req.size %d", file_req1.name, file_req1.size);

	/* open a file and send the first line */
	if((fic = fopen(file_req1.name, "r")) == NULL){
		perror("error filename");
		exit(-1);	 
	}

	nb_lu = fread(file_line, MAX_MSG, 1, fic);
	rc = send(sd, file_line, MAX_MSG, 0);
	printf("send of the first line of the file : %s\n", file_line);
	printf("size of the line : %d\n", nb_lu);
	printf("size of the line send : %d\n", rc);		
	
	for(i=2; i<argc; i++){
		rc = send(sd, argv[i],strlen(argv[i]) + 1, 0);
		
		if(rc<0){
			perror("cannot send data");
			close(sd);
			exit(1);
		}
		printf("%s: data%u sent (%s)\n", argv[0], i-1, argv[i]);
	}
	return 0;
}
