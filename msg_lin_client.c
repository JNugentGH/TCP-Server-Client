#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "5555" // In Windows the port must be a string.

#define MAXSIZE 512
#define BACKLOG 10   // how many pending connections queue will hold

int send_file(FILE *fp, int sockfd)
{
	int n;
	int total_n = 0;

	char data[MAXSIZE] = {0};
	//int data[MAXSIZE];

	//while(fgets(data, MAXSIZE, fp) != NULL)
	while(fread(data, sizeof(char), MAXSIZE, fp) > 0 )
	{
		n = send(sockfd, data, sizeof(data), 0);

		if(n == -1)
		{
			perror("Error Sending File.");

			exit(1);
		}

		printf("CLIENT: Sent %d Bytes.\n", n);
		
		total_n += n;

		memset(data, 0, MAXSIZE);
	}
	
	printf("CLIENT: Total Bytes Sent: %d\n", total_n);

	return 0;
}
int main(int argc, char *argv[])
{

	int sockfd, numbytes;

	char buf[MAXSIZE];

	struct addrinfo hints, *serverinfo, *p;

	int rv; // return-value for function calls.

	// Declare File Pointer.
	//FILE *fp;
	//char *filename;

	char message[MAXSIZE];

	if(argc!=2)
	{
		fprintf(stderr, "Program takes IP input argument\n");

		exit(1);
	}

	//filename = argv[2];

	// The hints for the type of socket we want needs to be cleared with zeros for most values.
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// return-value = getaddrinfo(hostname, port, addrinfo hints, addrinfo response)
	rv = getaddrinfo(argv[1], PORT, &hints, &serverinfo);

	if(rv != 0)
	{
		fprintf(stderr, "Getaddrinfo");

		return 1;
	}

	for(p = serverinfo; p != NULL; p -> ai_next)
	{
		// socket-file-descriptor = socket(SOCKET, sockaddr *name, int namelen);
		sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);

		if(sockfd == -1)
		{
			close(sockfd);

			perror("Client: Socket");

			continue;
		}

		rv = connect(sockfd, p -> ai_addr, p -> ai_addrlen);

		if (rv == -1)
		{
			close(sockfd);

			perror("Client: Connect");

			continue;
		}

		break;
	
	}

	if(p == NULL)
	{
		fprintf(stderr, "Client: Failed to Connect\n");

		return 2;
	}

	freeaddrinfo(serverinfo);
	
	//rv = send(sockfd, argv[2], strlen(argv[2]), 0);
	
	while(1)
	{
		// Prompt user input.
		printf("You:");
		fgets(message, MAXSIZE, stdin);

		// Send client intro.
		rv = send(sockfd, message, strlen(message), 0);
		
		if(rv ==-1)
		{
			perror("send");

			exit(1);
		}
		
		// Receive Server welcome Message.
		numbytes = recv(sockfd, buf, MAXSIZE-1, 0);

		if(numbytes == -1)
		{
			perror("recv");

			exit(1);
		}
		
		// Received zero bytes, the connection is now closed.
		else if(numbytes==0)
		{
			break;
		}

		// Add a '\0' after the last byte to terminate the string.
		buf[numbytes] ='\0';

		// Display the recieved message.
		printf("Them: %s \n", buf);
	}

	close(sockfd);

	return 0;
}