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

/* definition of the type file request */
typedef struct file_req_s {
   char name[20];
   int size;
   char zero[8]; /* pour que la requete fasse 32bits */
} file_req_t ;

/*function readline*/
extern int read_line(int newSd, char *line_to_return);

int main (int argc, char *argv[])
{
	int sd, newSd, n;
	socklen_t cliLen ;
	
	file_req_t file_req1;

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
		printf("size of the file : %d\n", file_req1.size);
		
		/* reception of the first line of the file */
		n = recv(newSd, &line, MAX_MSG, 0);
		printf("first line of the file recepted\n");
		printf("%s : %s\n", file_req1.name, line);

		/*receive segments*/
		while(read_line(newSd, line) != ERROR)
		{
			printf("%s : received from %s : TCP%d : %s\n", argv[0], inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), line);
			
			/*init line*/
			memset(line, 0x0, MAX_MSG) ;
		}/*while(read_line)*/
	}/*while(1)*/

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

