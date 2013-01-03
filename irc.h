
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

typedef struct irc_msg {
	char *source;
	char *command;
	int command_num;
	char *target;
	char *params;
} irc_msg;

/* High level IRC functions */
int irc_connect(irc_connection *con, char *hostname, int port);
void irc_close(irc_connection *con, char *qmsg);
int irc_set_nick(irc_connection *con, const char *nick);
int irc_set_user(irc_connection *con, 
		const char *username, const char *hostname,
		const char *servername, const char* realname);
int irc_messages_pending(irc_connection *con);
char* irc_next_message_rawstr(irc_connection *con);
irc_msg* irc_next_message(irc_connection *con);
void irc_free_msg(irc_msg *msg);
int wait_fill_buffer(irc_connection *con);
int irc_send_raw_msg(irc_connection *con, char *msg);

/* Low level */
int recv_string(irc_connection *con, char *buf, int maxlen);
int send_string(irc_connection *con, char *buf);

