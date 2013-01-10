#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "helpers.h"
#include "irc.h"
#include "config.h"
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
	unsigned irc_port;
	char *irc_server;

	config *conf = config_from_filename("ircb.conf");
	assert(conf);

	if (argc == 3) {
		irc_server = argv[1];
		irc_port = atoi(argv[2]);
	}
	if (argc == 2) {
		irc_server = argv[1];
		irc_port = 6667;
	}
	else if (argc == 1) {
		config_group *g = config_get_group(conf, "irc");
		irc_server = Conf("server", "INVALID");
		if (!strcmp(irc_server, "INVALID")) {
			Printerr("No IRC server address given!\n");
			return 1;
		}
		char *portstr = Conf("port", "6667");
		assert(irc_server && portstr);
		irc_port = atoi(portstr);
	}
	else
		return 1;

	err = irc_connect(&con, irc_server, irc_port);
	if (err) {
		Printerr("Got no connection to IRC server!\n");
		return 1;
	}

	module_load_module_dir(&con);

	config_group *g = config_get_group(conf, "bot");

	irc_set_nick(&con, Conf("nickname", "cbot"));
	irc_set_user(&con, 
			Conf("username", "cbot_user"), 
			Conf("hostname", "cbot_host"), 
			Conf("servername", "cbot_server"), 
			Conf("realname", "The CBot!"));

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
