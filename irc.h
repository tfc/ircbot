
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
} irc_connection;

/*
 * According to RFC 1359, every IRC message has
 * the same format. 
 * irc_next_message() returns you this struct
 * filled with the according substrings of the
 * last server message
 */
typedef struct irc_msg {
	char *source;
	char *command;
	int command_num;
	char *target;
	char *params;
} irc_msg;

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

