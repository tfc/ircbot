#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "irc.h"

#define IRC_BUFFER_SIZE 4096

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#if 0
static void dump_buffer(irc_connection *con)
{
	printf("{");
	for (int i=0; i < IRC_BUFFER_SIZE; i++) {
		if (i == con->rpos) printf("[");
		if (i == con->wpos) printf("]");

		if (con->buf[i] == '\n' || con->buf[i] == '\r') 
			printf("^");
		else if (con->buf[i] == 0)
			printf("_");
		else
			printf("%c", con->buf[i]);
	}
	printf("}\n");
}
#endif

int irc_connect(irc_connection *con, char *hostname, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	assert(con);
	assert(hostname);
	assert(port);

	con->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (con->sockfd < 0) {
		fprintf(stderr, "ERROR opening socket");
		return 1;
	}

	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		return 2;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(con->sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR connecting");
		return 3;
	}

	con->buf = malloc(IRC_BUFFER_SIZE+1);
	con->buf[IRC_BUFFER_SIZE] = '\0';
	con->wpos = con->rpos = 0;

	return 0;
}

void irc_close(irc_connection *con, char *qmsg)
{
	assert(con);

	char *msg;
	if (qmsg) asprintf(&msg, "QUIT :%s\n", qmsg);
	else msg = strdup("QUIT\n");
	send_string(con, msg);
	free(msg);

	free(con->buf);
	free(con->nick);
	free(con->username);
	free(con->hostname);
	free(con->servername);
	free(con->realname);
	close(con->sockfd);
}

int irc_set_nick(irc_connection *con, const char *nick)
{
	char *sendmsg;

	con->nick = strdup(nick);
	strcpy(con->nick, nick);

	asprintf(&sendmsg, "NICK %s\n", con->nick);
	if (!sendmsg) return 1;

	send_string(con, sendmsg);
	free(sendmsg);

	return 0;
}

int irc_set_user(irc_connection *con, 
		const char *username, const char *hostname,
		const char *servername, const char* realname)
{
	con->username = strdup(username);
	con->hostname = strdup(hostname);
	con->servername = strdup(servername);
	con->realname = strdup(realname);

	if (!con->username || !con->hostname || !con->servername || !con->realname) {
		free(con->username);
		free(con->hostname);
		free(con->servername);
		free(con->realname);
		return 1;
	}

	char *sendmsg;
	asprintf(&sendmsg, "USER %s %s %s :%s\n", 
			username, hostname, servername, realname);
	send_string(con, sendmsg);
	free(sendmsg);

	return 0;
}

static int ringbuf_free(irc_connection *con)
{
	int diff = con->wpos - con->rpos;
	assert(-diff <= IRC_BUFFER_SIZE);

	if (diff < 0)
		return -1 -diff;
	else
		return IRC_BUFFER_SIZE-1 -diff;
}

int irc_messages_pending(irc_connection *con)
{
	int msgs = 0;

	if (con->rpos <= con->wpos) {
		for (unsigned i = con->rpos; i < con->wpos; i++)
			if (con->buf[i] == '\n') msgs++;
	}
	else {
		for (unsigned i = con->rpos; i < IRC_BUFFER_SIZE; i++)
			if (con->buf[i] == '\n') msgs++;
		for (unsigned i = 0; i < con->wpos; i++)
			if (con->buf[i] == '\n') msgs++;
	}

	return msgs;

}

char* irc_next_message(irc_connection *con)
{
	char *p = con->buf + con->rpos;
	char *p2 = strchr(p, '\n');
	char *msg;

	if (!p2) {
		p2 = strchr(con->buf, '\n');
		*(p2-1) = '\0';
		asprintf(&msg, "%s%s", p, con->buf);
	}
	else {
		*(p2-1) = '\0';
		msg = strdup(p);
	}

	con->rpos = p2 + 1 - con->buf;

	return msg;
}

int wait_fill_buffer(irc_connection *con)
{
	unsigned free = ringbuf_free(con);
	unsigned right = IRC_BUFFER_SIZE - con->wpos;
	free = MIN(free, right);

	int recvd = recv_string(con, con->buf + con->wpos, free);
	con->wpos = (con->wpos + recvd) % IRC_BUFFER_SIZE;

	con->buf[con->wpos] = '\0';

	return recvd;
}

int recv_string(irc_connection *con, char *buf, int maxlen)
{
	int recvd;

	recvd = read(con->sockfd, buf, maxlen);
	if (recvd < 0) {
		fprintf(stderr, "ERROR reading from socket");
		return 0;
	}
	return recvd;
}

int send_string(irc_connection *con, char *buf)
{
	int n;

	printf("--> %s", buf);

	n = write(con->sockfd, buf, strlen(buf));
	if (n < 0) {
		fprintf(stderr, "ERROR writing to socket");
		return 0;
	}
	return n;
}
