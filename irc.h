#ifndef __IRC_H__
#define __IRC_H__

/* See the innards of irc_msg and irc_connections here. */
#include "irc_common_struct.h"

/* Connect to the IRC and initialize *con.
 * Returns 0 if everything went nice.
 */
int irc_connect(irc_connection *con, char *hostname, int port);

/* Close an IRC connection and clean up the data structures. */
void irc_close(irc_connection *con, char *qmsg);

/* Set the IRC nick name. */
int irc_set_nick(irc_connection *con, const char *nick);

/* Set the IRC user and his user data */
int irc_set_user(irc_connection *con, 
		const char *username, const char *hostname,
		const char *servername, const char* realname);

/* Provide a raw message string and send it to the server. */
int irc_send_raw_msg(irc_connection *con, char *msg);

/* Block on the IRC socket until data arrives.
 * Returns 0 on disconnect
 */
int wait_fill_buffer(irc_connection *con);

/* Returns the number of messages in the buffer available for handling */
int irc_messages_pending(irc_connection *con);

/* Pop off and parse the last IRC message from the buffer. */
irc_msg* irc_next_message(irc_connection *con);

/* Pop off the last raw IRC message string from the buffer */
char* irc_next_message_rawstr(irc_connection *con);

/* Free an irc_msg structure after use. */
void irc_free_msg(irc_msg *msg);

/* Low level */
int recv_string(irc_connection *con, char *buf, int maxlen);
int send_string(irc_connection *con, char *buf);

#endif /* __IRC_H__ */

