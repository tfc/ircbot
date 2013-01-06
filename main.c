#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "helpers.h"
#include "irc.h"
#include "module_support.h"


static int handle_keyboard_input(irc_connection *con)
{
	char msg[512];
	char *ret = fgets(msg, 511, stdin);
	if (!ret) return -1;

	/* user input "q" quits the whole application. */
	if (!strcmp(msg, "q\n")) return -2;

	return irc_send_raw_msg(con, msg);
}

static int handle_irc_messages(irc_connection *con)
{
	wait_fill_buffer(con);
	int msgs = irc_messages_pending(con);

	while (msgs--) {
		irc_msg *msg = irc_next_message(con);
		if (!msg) { 
			printf("Erroneous message.\n"); 
			continue; 
		}
		if (!module_handle_msg(con, msg))
#if 0
			printf("<-- %s\n", msg->raw_str);
#else
			printf("<-- SRC %s CMD %s TGT %s PRM %s\n", 
					msg->source, msg->command, msg->target, msg->params);
#endif
		irc_free_msg(msg);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int err;
	int max_descr;
	fd_set sock_set;
	struct timeval select_timeout;
	int running = 1;
	irc_connection con;
	unsigned port;

	if (argc == 2) 
		/* No port provided from command line. 
		 * Using default port.
		 */
		port = 6667;
	else if (argc != 3) {
		printf("Usage: %s <IRC server> <port>\n", argv[0]);
		return 1;
	}
	else
		port = atoi(argv[2]);

	err = irc_connect(&con, argv[1], port);
	if (err) {
		fprintf(stderr, "Got no connection.");
		exit(1);
	}

	module_load_module_dir(&con);

	irc_set_nick(&con, "cbot");
	irc_set_user(&con, "cbot_user", "cbot_host", "cbot_servername", "CBot Real Name");

	max_descr = MAX(STDIN_FILENO, con.sockfd) + 1;

	while (running) {
		FD_ZERO(&sock_set);
		FD_SET(STDIN_FILENO, &sock_set);
		FD_SET(con.sockfd, &sock_set);

		select_timeout.tv_sec = 120;
		select_timeout.tv_usec = 0;

		err = select(max_descr, &sock_set, NULL, NULL, &select_timeout);

		if (!err) {
			/* Handle timeout */
		}

		if (FD_ISSET(con.sockfd, &sock_set))
			/* Incoming data from IRC network */			
			running = !handle_irc_messages(&con);

		if (FD_ISSET(STDIN_FILENO, &sock_set))
			/* Local user input */
			running = handle_keyboard_input(&con) != -2;
	}

	irc_close(&con, "bye.");
	return 0;
}
