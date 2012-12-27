#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "irc.h"

int connect_to_irc(char *hostname, int port)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	assert(hostname);
	assert(port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "ERROR opening socket");
		return 0;
	}

	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		return 0;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR connecting");
		return 0;
	}

	return sockfd;
}

int recv_string(int sockfd, char *buf)
{
	int n;
	n = read(sockfd, buf, 512);
	if (n < 0) {
		fprintf(stderr, "ERROR reading from socket");
		return 0;
	}
	return n;
}

int send_string(int sockfd, char *buf)
{
	int n;
	n = write(sockfd, buf, strlen(buf));
	if (n < 0) {
		fprintf(stderr, "ERROR writing to socket");
		return 0;
	}
	return n;
}
