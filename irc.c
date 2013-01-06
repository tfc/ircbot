#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <glib.h>

#include "helpers.h"
#include "irc.h"
#include "irc_codes.h"

#define IRC_BUFFER_SIZE 4096

#define Irc_send(con, ...) do { \
	char *sendmsg; \
	asprintf(&sendmsg, __VA_ARGS__); \
	send_string(con, sendmsg); \
	free(sendmsg); \
} while(0)

#define Copy_match(__info, __num, __dest) do { \
	gchar *word = g_match_info_fetch(__info, __num); \
	__dest = strdup(word); \
	g_free(word); \
} while (0)

static int irc_connections = 0;
static GRegex *irc_msg_regex_pattern = NULL;
static GRegex *irc_msgsrc_regex_pattern = NULL;

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

	if (!irc_msg_regex_pattern)
		irc_msg_regex_pattern = g_regex_new(
#if 0
			/* Found this pattern at 
			 * http://calebdelnay.com/blog/2010/11/parsing-the-irc-message-format-as-a-client
			 */
			"(?::(\\S+) )?(\\S+)(?: (?!:)(.+?))?(?: :(.+))?$",
#else
			/* And this one somewhere else (some thread on stackoverflow.com). 
			 * Will add the URL when i have found it... */
			"^(?:[:@]([^\\s]+) )?([^\\s]+)(?: ((?:[^:\\s][^\\s]* ?)*))?(?: ?:(.*))?$",
#endif
			0, 0, NULL);
	if (!irc_msgsrc_regex_pattern)
		irc_msgsrc_regex_pattern = g_regex_new("^(.+?)!(.+?)@(.+?)$", 0, 0, NULL);

	assert(irc_msg_regex_pattern);
	assert(irc_msgsrc_regex_pattern);

	irc_connections++;

	con->modules = NULL;

	return 0;
}

void irc_close(irc_connection *con, char *qmsg)
{
	assert(con);

	if (qmsg) Irc_send(con, "QUIT :%s\n", qmsg);
	else Irc_send(con, "QUIT\n");

	Free_list(con->buf, con->nick, con->username, con->hostname, con->servername, con->realname);
	close(con->sockfd);

	if (!--irc_connections) {
		g_regex_unref(irc_msg_regex_pattern);
		g_regex_unref(irc_msgsrc_regex_pattern);
	}

	assert(irc_connections >= 0);
}

int irc_set_nick(irc_connection *con, const char *nick)
{
	con->nick = strdup(nick);
	Irc_send(con, "NICK %s\n", con->nick);
	return 0;
}

int irc_set_user(irc_connection *con, 
		const char *username, const char *hostname,
		const char *servername, const char* realname)
{
	con->username   = strdup(username);
	con->hostname   = strdup(hostname);
	con->servername = strdup(servername);
	con->realname   = strdup(realname);

	if (!con->username || !con->hostname || !con->servername || !con->realname) {
		Free_list(con->username, con->hostname, con->servername, con->realname);
		return 1;
	}

	Irc_send(con, "USER %s %s %s :%s\n", username, hostname, servername, realname);

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
    unsigned i;

	if (con->rpos <= con->wpos) {
		for (i = con->rpos; i < con->wpos; i++)
			if (con->buf[i] == '\n') msgs++;
	}
	else {
		for (i = con->rpos; i < IRC_BUFFER_SIZE; i++)
			if (con->buf[i] == '\n') msgs++;
		for (i = 0; i < con->wpos; i++)
			if (con->buf[i] == '\n') msgs++;
	}

	return msgs;
}

char* irc_next_message_rawstr(irc_connection *con)
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

irc_msg* irc_next_message(irc_connection *con)
{
	gboolean ret;
	GMatchInfo *msg_info;
	GMatchInfo *user_info;
	irc_msg *ircmsg = NULL;
	char *raw_msg = irc_next_message_rawstr(con);

	if (!raw_msg) goto ircmsg_out;

	ret = g_regex_match(irc_msg_regex_pattern, raw_msg, 0, &msg_info);
	if (!ret) goto ircmsg_out;

	int matches = g_match_info_get_match_count(msg_info);
	if (matches != 5 && matches != 4) goto ircmsg_out;

	ircmsg = malloc(sizeof(irc_msg));
	if (!ircmsg) goto ircmsg_out;

	ircmsg->raw_str = strdup(raw_msg);
	Copy_match(msg_info, 1, ircmsg->source);
	Copy_match(msg_info, 2, ircmsg->command);
	Copy_match(msg_info, 3, ircmsg->target);
	if (matches == 5) Copy_match(msg_info, 4, ircmsg->params);
	else 		  ircmsg->params = NULL;

	ret = g_regex_match(irc_msgsrc_regex_pattern, ircmsg->source, 0, &user_info);
	if (!ret) goto ircmsg_userinfo_out;

	matches = g_match_info_get_match_count(user_info);
	if (matches != 4) goto ircmsg_userinfo_out;

	Copy_match(user_info, 1, ircmsg->src_nick);
	Copy_match(user_info, 2, ircmsg->src_user);
	Copy_match(user_info, 3, ircmsg->src_host);

	goto ircmsg_out;

ircmsg_userinfo_out:
	ircmsg->src_nick = ircmsg->src_user = ircmsg->src_host = NULL;
ircmsg_out:
	g_match_info_free(msg_info);
	g_match_info_free(user_info);
	return ircmsg;
}

void irc_free_msg(irc_msg *msg)
{
	Free_list(msg->raw_str, 
		msg->source, msg->command, msg->target, msg->params, 
		msg->src_nick, msg->src_user, msg->src_host,
		msg);
}

int irc_send_raw_msg(irc_connection *con, char *msg)
{
	assert(con);
	assert(msg);

	if (msg[strlen(msg)-1] != '\n') return 0;

	return send_string(con, msg);
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

