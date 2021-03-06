/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 1024	

pthread_t tid[MAXDATASIZE];

char **res;
int numbytes; 

struct thread_info {    /* Used as argument to thread_start() */
           int       thread_num;       /* Application-defined thread # */
		   int       fd;
           char     *argv_string;      /* From command-line argument */
       };

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sendGoodRequest(int sockfd, char *input)
{
	char *newInput=input;
	int counter = 0;
	if (input[0]=='/')
		newInput=&input[1];
	unsigned char buf[MAXDATASIZE];
	unsigned char garbage_buf[MAXDATASIZE];
	
	FILE *pf;
	pf = fopen(newInput, "rb");
	int fileSize;
	if(pf)
	{
		sprintf(buf, "HTTP/1.0 200 OK\r\n\r\n");
		counter += send(sockfd, buf, strlen(buf),0);

		while(1)
	{
		fileSize=fread(buf, 1, 1, pf);
		
		if(fileSize<=0)
		{
			break;
		}
		
		else {
			counter += send(sockfd, buf, fileSize, 0);
		}
	}
	printf("%d\n", counter);
	fclose(pf);

	}	
	else{
		sprintf(buf, "HTTP/1.0 404 Not Found\r\n\r\n");
		send(sockfd, buf, strlen(buf), 0);
	}
	recv(sockfd, &garbage_buf, MAXDATASIZE, 0);

}

void sendBadRequest(int sockfd)
{
	char buf[MAXDATASIZE];
	sprintf(buf, "HTTP/1.0 400 Bad Request\r\n\r\n");
	send(sockfd, buf, strlen(buf),0);
}

void* mainEvent(void *arg)
{
		int index=0;
		int counter=0;
		int new_fd;
		int thread_num;

		char temp=0;
		char temp1=0;
		char temp2=0;
		char temp3=0;
		char temp4=0;

		struct thread_info *tinfo = arg;
		new_fd = tinfo->fd;
		thread_num = tinfo->thread_num;
		printf("fd number is %d\n",new_fd );

		pthread_t id = pthread_self();

    printf("%d thread processing\n", thread_num );



		while(1)
	{
		if((numbytes = recv(new_fd, &temp, 1, 0)) <= 0)
		{
			break;
		}
		// printf("%d\n", numbytes);
		// printf("%c\n",temp);

		temp1=temp2;
		temp2=temp;
		//printf("%i, %i\n", counter, index );
		//printf("%i\n",temp );

		res[counter][index]=temp;

		if((temp1=='\r') && (temp2=='\n'))
		{
			res[counter][index-1]='\0';
			counter++;
			break;
		}


		index++;

	}

	char *secondString;

	if(res[0][0]=='G' && res[0][1]=='E' && res[0][2] =='T')
	{
		if(res[0][3]==' ')
		{
			secondString = strchr(&res[0][4], ' ');
			if(secondString==NULL)
			{
				//printf("going to send bad request");
				sendBadRequest(new_fd);
			}
			else{
				//printf("going to send good request");
				secondString[0]='\0';
				sendGoodRequest(new_fd, &res[0][4]);
				printf("returned from send good request\n");
			}
		}
	}


	close(new_fd);
}


int main(int argc, char* argv[])
{
	 // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int sockfd;
	int new_fd;

	struct thread_info *tinfo;

	if (argc != 2) {
	   fprintf(stderr,"usage: host port\n");
	   exit(1);
	}

	int i;
	int threadNum=0;
	
	res = malloc( MAXDATASIZE * sizeof(char*));
	for(i=0; i<MAXDATASIZE; i++)
	{
		res[i]=malloc( MAXDATASIZE * sizeof(char));
	}


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP



	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");


	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			//perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		// if (!fork()) { // this is the child process
		// 	close(sockfd);
		// }

		/* Allocate memory for pthread_create() arguments */

        tinfo = calloc(MAXDATASIZE, sizeof(struct thread_info));


		tinfo[threadNum].thread_num = threadNum;
        tinfo[threadNum].fd = new_fd;
        printf("%d\n", new_fd);

		pthread_create(&(tid[threadNum]), NULL, &mainEvent, &tinfo[threadNum] );
		threadNum++;

	}
	//close(new_fd);
	exit(0);

	return 0;
}


	

	