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

/*
 * A handy helper macro for tayloring a string and sending it
 * to the IRC server immediately:
 * Irc_send(con, "my %s string nr. %d\n", "favorite", x);
 * --> Don't forget to append a \n to every message!
 */
#define Irc_send(con, ...) do { \
	char *sendmsg; \
	asprintf(&sendmsg, __VA_ARGS__); \
	irc_send_raw_msg(con, sendmsg); \
	free(sendmsg); \
} while(0)

/*
 * Another handy helper for copying substring matches from 
 * a gregex info structure. 
 * The string is on your heap - free it yourself after use.
 * 
 * Example:
 *   g_regex_match(my_regex_pattern, my_string, 0, &my_info_var);
 *   Copy_match(my_info_var, <group nr>, my_destination_string_ptr);
 */
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

	/* Optional quit message */
	if (qmsg) Irc_send(con, "QUIT :%s\n", qmsg);
	else 	  Irc_send(con, "QUIT\n");

	Free_list(con->buf, con->nick, con->username, con->hostname, 
		con->servername, con->realname);
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

/* Returns how many bytes on the ring buffer are writable. */
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

	/* Just counting how many \n there are between read-
	 * and write-position of the ring buffer.
	 * Every IRC message is terminated with \n.
	 */
	if (con->rpos <= con->wpos) {
		for (i = con->rpos; i < con->wpos; i++)
			if (con->buf[i] == '\n') msgs++;
	}
	else {
		/* Ring buffer is currently wrapping around. */
		for (i = con->rpos; i < IRC_BUFFER_SIZE; i++)
			if (con->buf[i] == '\n') msgs++;
		for (i = 0; i < con->wpos; i++)
			if (con->buf[i] == '\n') msgs++;
	}

	return msgs;
}

char* irc_next_message_rawstr(irc_connection *con)
{
	/* To return the next irc message string, we need
	 * to know where the next \n is. 
	 * So we start looking for \n at the reader position.
	 */
	char *p = con->buf + con->rpos;

	/* This search is not dangerous when there is a wrap-around
	 * in the ring buffer, because the first byte just behind
	 * the buffer space is set to \0.
	 */
	char *p2 = strchr(p, '\n');
	char *msg;

	if (!p2) {
		/* Didn't find an \n. Continue the
		 * search at the beginning of the buffer.
		 * You should not have called this function if 
		 * irc_messages_pending(..) returned 0, because this will
		 * break here right now. */
		p2 = strchr(con->buf, '\n');

		/* Terminate the string one character before, because every
		 * message is terminated with \r\n.
		 */
		*(p2-1) = '\0';

		/* Combine the two wrap-around pieces into one string. */
		asprintf(&msg, "%s%s", p, con->buf);
	}
	else {
		*(p2-1) = '\0';
		msg = strdup(p);
	}

	/* Push the read position pointer one message further. */
	con->rpos = p2 + 1 - con->buf;

	/* Returning a valid null-terminated string. */
	return msg;
}

irc_msg* irc_next_message(irc_connection *con)
{
	gboolean ret;
	GMatchInfo *msg_info;
	GMatchInfo *user_info;
	irc_msg *ircmsg = NULL;

	/* We're getting a raw string from the ring buffer
	 * and parse it to provide the user a nice irc_msg structure. 
	 */
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

	/* If the source string has the format "nick!user@host" then
	 * we parse further to provide the user more substrings.
	 */
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

	/* Refuse sending messages which are not \n terminated.
	 * The IRC server would not handle it as a message because
	 * it would be waiting for the next \n.
	 */
	if (msg[strlen(msg)-1] != '\n') return 0;

	return send_string(con, msg);
}

int wait_fill_buffer(irc_connection *con)
{
	unsigned free = ringbuf_free(con);
	unsigned right = IRC_BUFFER_SIZE - con->wpos;
	/* In the wrap-around case write only to the end of the
	 * ring buffer. This function needs to be called 
	 * again to write from the beginning after that.
	 */
	free = MIN(free, right);

	int recvd = recv_string(con, con->buf + con->wpos, free);

	/* Move the write-position behind the new message content. */
	con->wpos = (con->wpos + recvd) % IRC_BUFFER_SIZE;

	/* String-terminate the buffer here. */
	con->buf[con->wpos] = '\0';

	/* If returning 0 here, the caller needs to realize that
	 * the server closed the connection!
	 */
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

