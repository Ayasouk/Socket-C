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

#define REQ_OK 1
#define REQ_UPLOAD 2
#define REQ_KO -1

/* definition of the type file request */
typedef struct file_req_s {
   char name[20];
   int size;
   char REQ;
} file_req_t ;

int read_sizef(char * filename);

int main(int argc, char ** argv){
	int sd, rc, i;
	struct sockaddr_in localAddr, servAddr;
	struct hostent *h;
	
	/* data linked with the file */
	file_req_t file_req1;
	FILE * fic = NULL;	
	char file_line[MAX_MSG];
	int cpt_line = 0; /* to print the number of line sent */	

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
	file_req1.size = read_sizef(argv[2]);
	file_req1.REQ = REQ_UPLOAD; /* requete d'envoi de fichier */

 	/* send of a file request */
	rc = send(sd, &file_req1, sizeof(file_req_t), 0);
	printf(" send a file request to %s\n", argv[1]);
	printf("file_req.name : %s\n file_req.size %d\n ", file_req1.name, file_req1.size);

	/* reception of the file request to comfirm */
	rc = recv(sd, &file_req1, sizeof(file_req_t), 0);
	(file_req1.REQ == REQ_OK)? printf("reception of the comfirmation :\n") : perror("the transfer cancel by the server");

	/* open a file and send the first line */
	if((fic = fopen(file_req1.name, "r")) == NULL){
		perror("error : opening file");
		exit(-1);	 
	}
	
	/* send of the file line by line*/
	while(fgets(file_line, MAX_MSG, fic) != NULL){
		rc = send(sd, file_line, MAX_MSG, 0);
		cpt_line++;
		printf("line n°%d : %s\n", cpt_line, file_line);
		printf("size of the line send : %d\n", rc);	
	}
	
	fclose(fic); /* close the file */
	close(sd);
	return 0;
}

int read_sizef(char * filename){
	int size;
	FILE * fic = NULL;
	
	if((fic = fopen(filename, "r")) == NULL){
		perror("error : opening file");
		exit(-1);	
	}
	fseek(fic, 0, SEEK_END);
	size = ftell(fic);
	fseek(fic, 0, SEEK_SET);
	fclose(fic);
	return size;
}

