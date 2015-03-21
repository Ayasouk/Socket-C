#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /*close*/

#include "common.h"

typedef struct file_req_s {  /* definition of the type file request */
   char name[20];
   int size;
   char REQ; /* REQ_UPLOAD, REQ_OK or REQ_KO */
} file_req_t ;


int main (int argc, char *argv[])
{
	int sd, newSd, n; 
	socklen_t cliLen ;
	
	/* data concerning the file */
	file_req_t file_req1;	
	char choice; 
	char rep_file[64];
	FILE * fic = NULL;

	struct sockaddr_in cliAddr, servAddr ;
	char line[MAX_MSG] ;
	
	/*create socket*/
	sd = socket(AF_INET, SOCK_STREAM, 0) ;
	if(sd < 0)
	{
		perror("Cannot open socket !") ;
		return ERROR ;
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
		printf("cliaddr.sin_family : %d", cliAddr.sin_family);
		if(newSd < 0)
		{
			perror("Cannot accept connection !") ;
			return ERROR ;
		}
		
		/*init line*/
		memset(line, 0x0, MAX_MSG) ;
		
		/* reception of a file request */
 		n = recv(newSd, &file_req1, sizeof(file_req1), 0);
		printf("file request recepted \n filename : %s\n", file_req1.name);
		printf("size of the file : %d octets\n", file_req1.size);
		
		if(file_req1.REQ == REQ_UPLOAD){
			printf(" Do you want to receive the file (%d octets)? Y(es)/N(o)", file_req1.size);
			scanf("%c", &choice);
			switch(choice){
				case 'Y' : file_req1.REQ = REQ_OK;
					 n = send(newSd, &file_req1, sizeof(file_req_t), 0); /* send a comfirmation that it can receive the file */
					 break;
				case 'N' : file_req1.REQ = REQ_KO; 
					 n = send(newSd, &file_req1, sizeof(file_req_t), 0);
					 break;
				case '?' : perror("error invalid answer");
					 return ERROR;
			}		 
		} 

		if(file_req1.REQ == REQ_OK){ /* if the server accpet the file , reception of the file */
			printf("BEGIN : reception of the file %s \n", file_req1.name);
			/* create a new file which will be a copy of the sent file */	
			sprintf( rep_file, "tmp local/%s", file_req1.name); /* to open in the repertory tmp local */
			
			if((fic = fopen(rep_file, "w")) == NULL){
				perror("error : opening file");
				return ERROR;		
			}

			/* reception of the file line by line */
			memset(line, 0x0, MAX_MSG); /* init buffer */
			fprintf(fic ,"test ");
			
			while((n = recv( newSd, line, MAX_MSG, 0)) > 0){
				
				if( fwrite(line, MAX_MSG, 1, fic) < 0){  /* write the line on the file */
					perror("error : writing on file");
					return ERROR;	
				}				
				memset(line, 0x0, MAX_MSG);
			}
			printf("END : file reception %s\n", file_req1.name);
			if(n == 0){
				perror("Connection closed by the Client\n");
				return ERROR;		
			}
			if(n < 0){
				perror("Cannot receive data\n");
				return ERROR;		
			}
			
		}
		
	}/*while(1)*/
	close(sd);
	close(newSd);
	fclose(fic);
}


