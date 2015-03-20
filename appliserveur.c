#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /*close*/

#define SUCCESS 0
#define ERROR 1

#define END_LINE 0x0
#define SERVER_PORT 1500
#define MAX_MSG 100

#define REQ_OK 1
#define REQ_UPLOAD 2
#define REQ_KO -1

/* definition of the type file request */
typedef struct file_req_s {
   char name[20];
   int size;
   char REQ; /* SND, OK or KO */
} file_req_t ;

/*function readline*/
extern int read_line(int newSd, char *line_to_return);

int main (int argc, char *argv[])
{
	int sd, newSd, n; 
	socklen_t cliLen ;
	
	/* data concerning the file */
	file_req_t file_req1;
	FILE * fic = NULL;	
	int cpt_line = 0; /* to print the number of the line */
	char rep_file[64];
	char choix; 

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
			scanf("%c", &choix);
			switch(choix){
				case 'Y' : file_req1.REQ = REQ_OK;
					 n = send(newSd, &file_req1, sizeof(file_req_t), 0);
					 break;
				case 'N' : file_req1.REQ = REQ_KO;
					 n = send(newSd, &file_req1, sizeof(file_req_t), 0);
					 break;
				case '?' : perror("error invalid answer");
					 return ERROR;
			}		 
		} 


		/* create a new file which will be a copy of the sent file */
		sprintf( rep_file, "tmp local/%s", file_req1.name); 
		if((fic = fopen(rep_file, "w")) == NULL){
			perror("error : opening file");
			return ERROR;		
		}
	
		/* reception of the file line by line */
		memset(line, 0x0, MAX_MSG); /* init buffer */
		while((n = recv(newSd, line, MAX_MSG, 0)) > 0){
			cpt_line++;
			
			if( fprintf(fic, "%s", line) < 0){  /* write the line on the file */
				perror("error : writing on file");
				return ERROR;	
			}				
			printf("line n°%d : %s\n", cpt_line, line); 
			memset(line, 0x0, MAX_MSG);
		}
		if(n == 0){
			perror("Connection closed by the Client\n");
			return ERROR;		
		}
		if(n < 0){
			perror("Cannot receive data\n");
			return ERROR;		
		}
		
	}/*while(1)*/
	fclose(fic); /* close the file */
	close(newSd);
}
	
	int read_line(int newSd, char *line_to_return)
	{
		static int rcv_ptr = 0 ;
		static char rcv_msg[MAX_MSG] ;
		static int n ;
		int offset ;
		
		offset = 0 ;
		
		while(1)
		{
			if(rcv_ptr == 0)
			{
				/*read data from socket*/
				memset(rcv_msg, 0x0, MAX_MSG) ; /*init buffer*/
				n = recv(newSd, rcv_msg, MAX_MSG, 0) ; /*wait for data*/
				if(n < 0)
				{
					perror("Cannot receive data !") ;
					return(ERROR) ;
				}
				if(n == 0)
				{
					printf("Connection closed by client\n") ;
					close(newSd) ;
					return(ERROR) ;
				}
			}
			
			/*if new data read on socket*/
			/*OR*/
			/*if another line is still in buffer*/
			
			/*copy line into 'line_to_return'*/
			while(*(rcv_msg+rcv_ptr) != END_LINE && rcv_ptr < n)
			{
				memcpy(line_to_return+offset, rcv_msg+rcv_ptr, 1) ;
				offset++ ;
				rcv_ptr++ ;
			}
			
			/*end of line + end of buffer => return line*/
			if(rcv_ptr == n-1)
			{
				/*set last byte to END LINE*/
				*(line_to_return+offset) = END_LINE ;
				rcv_ptr = 0 ;
				return ++offset ;
			}
			
			/*end of line but still some data in buffer*/
			if(rcv_ptr < n-1)
			{
				/*set last byte to END LINE*/
				*(line_to_return+offset) = END_LINE ;
				rcv_ptr++ ;
				return ++offset ;
			}
			
			/*end of buffer but line is not ended*/
			/*wait for more data to arrive on the socket*/
			if(rcv_ptr == n)
			{
				rcv_ptr = 0 ;
			}
		}/*while*/
	}

