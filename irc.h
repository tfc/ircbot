
typedef struct {
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

/* High level IRC functions */
int irc_connect(irc_connection *con, char *hostname, int port);
void irc_close(irc_connection *con);
int irc_set_nick(irc_connection *con, const char *nick);
int irc_set_user(irc_connection *con, 
		const char *username, const char *hostname,
		const char *servername, const char* realname);
int irc_messages_pending(irc_connection *con);
char* irc_next_message(irc_connection *con);

/* Low level */
int recv_string(irc_connection *con, char *buf, int maxlen);
int wait_fill_buffer(irc_connection *con);
int send_string(irc_connection *con, char *buf);
