#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "irc.h"

int main(int argc, char *argv[])
{
	int sockfd;
	char buffer[512];

	sockfd = connect_to_irc(argv[1], atoi(argv[2]));
	if (sockfd <= 0) {
		fprintf(stderr, "Got no connection.");
		exit(1);
	}

	send_string(sockfd, "NICK cbot\n");
	send_string(sockfd, "USER cbot_user cbot_host cbot_servername :CBot Real Name\n");

	recv_string(sockfd, buffer);
	printf("%s\n",buffer);

	/*
	printf("Please enter a message: ");
	fgets(buffer, 512, stdin);
	send_string(sockfd, buffer);
	*/
	
	close(sockfd);
}
