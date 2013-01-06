#ifndef __IRC_MSG_STRUCT_H__
#define __IRC_MSG_STRUCT_H__

/*
 * This struct is an IRC connection handle.
 * Every irc_* function/procedure takes such 
 * a handle as the first parameter.
 *
 * Initialization:
 * 	irc_connection con;
 * 	err = irc_connect(&con, "my.server.com", 6667));
 *
 */
typedef struct irc_connection {
	int sockfd;

	char *buf;
	unsigned rpos;
	unsigned wpos;

	char *nick;
	char *username;
	char *hostname;
	char *servername;
	char *realname;

	void *modules;
} irc_connection;

/*
 * According to RFC 1359, every IRC message has
 * the same format. 
 * irc_next_message() returns you this struct
 * filled with the according substrings of the
 * last server message
 */
typedef struct irc_msg {
	char *raw_str;
	int command_num;

	char *source;
	char *command;
	char *target;
	char *params;

	/* These strings are actually substrings of
	 * irc_msg::source if it has a format like
	 * "nick!user@host".
	 * They are set to NULL if this is not the case. 
	 */
	char *src_nick;
	char *src_user;
	char *src_host;
} irc_msg;

#endif /* __IRC_MSG_STRUCT_H__ */

