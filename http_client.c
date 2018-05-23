/*
** http_client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void manageInput(char *input, char **res)
{
	if(strlen(input)<7)
		return;
	
	char standardInput[]="http://";

	int i;
	for(i=0; i<7; i++)
	{
		if(input[i]!=standardInput[i])
			return;
	}

	int inputLength=strlen(input);
	int index1, index2;

	char *realContent = &input[7];
	//printf("%s\n",realContent);
	const char symbol1=':';
	const char symbol2='/';

	for(i=0; i<strlen(realContent); i++)
	{
		if(realContent[i]==':')
		{
			index1=i;
			break;
		}
	}

	for(i=0; i<strlen(realContent); i++)
	{
		if(realContent[i]=='/')
		{
			index2=i;
			break;
		}
	}

	//printf("%zu\n", strlen(realContent));

	char *pointer1 = strchr(realContent,symbol1);
	char *pointer2 = strchr(realContent,symbol2);

	if(pointer1!=NULL && pointer2!=NULL)
	{
		memcpy(res[0], &realContent[0], index1);
		memcpy(res[1], &realContent[index1+1], index2-index1-1);
		memcpy(res[2], &realContent[index2],inputLength-index2-7);
		res[0][index1]='\0';
		res[1][index2-index1-1]='\0';
		res[2][inputLength-index2-7]='\0';
		//printf("%s\n%s\n%s\n",res[0],res[1],res[2]);
	}

	else if(pointer1==NULL && pointer2!=NULL)
	{
		memcpy(res[0], &realContent[0], index2);
		res[1]="80";
		memcpy(res[2], &realContent[index2],inputLength-index2-7);
		res[0][index2]='\0';
		res[2][inputLength-index2-7]='\0';
		//printf("%s\n%s\n%s\n",res[0],res[1],res[2]);
	}

	else if(pointer1!=NULL && pointer2==NULL)
	{
		memcpy(res[0], &realContent[0], index1);
		memcpy(res[1], &realContent[index1+1], inputLength-index1-8);
		res[2]="/index.html";
		res[0][index1]='\0';
		res[1][inputLength-index1-8]='\0';
		//printf("%s\n%s\n%s\n",res[0],res[1],res[2]);
	}

	else if(pointer1==NULL && pointer2==NULL)
	{
		memcpy(res[0], &realContent[0], inputLength-7);
		res[1]="80";
		res[2]="/index.html";
		res[0][inputLength-7]='\0';
		//printf("%s\n%s\n%s\n",res[0],res[1],res[2]);
	}

}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	/*if (argc != 2) {
	   fprintf(stderr,"usage: client hostname\n");
	   exit(1);
	}*/
	int i;
	char **res;
	res=malloc( 3 * sizeof(char*));
	for(i=0; i<3; i++)
	{
		res[i]=malloc( MAXDATASIZE * sizeof(char));
	}

	char **headerBuffer;
	headerBuffer=malloc( 100 * sizeof(char*));

	for(i=0; i<100; i++)
	{
		headerBuffer[i]=malloc( MAXDATASIZE * sizeof(char));
	}

	manageInput(argv[1],res);


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(res[0], res[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	//printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	sprintf(buf, "GET %s HTTP/1.0\r\n" 
			"User-Agent: Wget/1.15 (linux-gnu)\r\n"
			"Host: %s:%s\r\n\r\n", res[2], res[0], res[1]);
	//printf("%s\n", buf);

	send(sockfd, buf, strlen(buf), 0);

	int j;
	/*for(j=0;j<3;j++)
	{
		free(res[j]);
	}*/


	/*if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	   perror("recv");
	   exit(1);
	}*/

	   int total_counter = 0;
	   int counter=0;
	   unsigned char recv_buf[MAXDATASIZE];
	unsigned char temp=0;
	char temp1=0;
	char temp2=0;
	char temp3=0;
	char temp4=0;
	bool flag=false;
	int index=0;
	FILE *fp;
	fp=fopen("output", "w+");

	while(1)
	{
		if(!flag)
		{
			if((numbytes = recv(sockfd, &temp, 1, 0)) <= 0)
				break;
		}
		else
		{
			if((numbytes = recv(sockfd, recv_buf, MAXDATASIZE-1, 0)) <= 0)
				break;
		}
		//printf("%02x For test\n", temp);
		total_counter += numbytes;

		temp1=temp2;
		temp2=temp3;
		temp3=temp4;
		temp4=temp;

		if(flag)
		{
			//fputc(temp,fp);
			//printf("%d\n",numbytes );
			fwrite(recv_buf, sizeof(char), numbytes, fp);
		}

		if(!flag)
		{
			//printf("%i %i\n", counter, index);
			headerBuffer[counter][index]=temp;
			if(temp3=='\r' && temp4=='\n')
			{
				headerBuffer[counter][index-1]='\0';
				counter++;
				index=0;
			}
			else
				index++;
		}
		
		if((temp1==0x0d) && (temp2==0x0a) && (temp3==0x0d) && (temp4==0x0a))
		{
			flag =true;
		}

	}


	printf("%d\n", total_counter);

	fclose(fp);
	

	const char needle[10]="301";

	if(strstr(headerBuffer[0],needle)!=NULL)
	{
		headerBuffer[0]="http_client";
		headerBuffer[1]=&headerBuffer[1][10];
		printf("%s\n",headerBuffer[1] );
		main(2,headerBuffer);
	}

	//buf[numbytes] = '\0';


	//printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}