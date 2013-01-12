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

config *conf = NULL;

#define Is_cmd(__cmd) (!strcmp(cmd, __cmd))

static int handle_keyboard_input(irc_connection *con)
{
	char msg[512];
	int retval = 0;
	char *ret = fgets(msg, 511, stdin);
	if (!ret) return -1;

	char *parse = strdup(msg);
	char *cmd = strtok(parse, " \n");

	if (Is_cmd("q")) {
		/* user input "q" quits the whole application. */
		retval = -2;
	}
	else if (Is_cmd("load")) {
		char *modname = strtok(NULL, " \n");
		printf("Loading module %15s ... ", modname);

		int mod_ret = module_load(con, conf, modname);
		if (!mod_ret) printf("OK!\n");
		else	      printf("Error (%d)!\n", mod_ret);
	}
	else if (Is_cmd("unload")) {
		char *modname = strtok(NULL, " \n");
		printf("Unloading module %15s ... ", modname);

		int mod_ret = module_unload(con, modname);
		if (!mod_ret) printf("OK!\n");
		else	      printf("Error (%d)!\n", mod_ret);
	}
	else if (Is_cmd("modules")) {
		printf("Currently loaded modules:\n");
		char **modules = module_loaded_modules(con);
		int i;
		for (i=0; modules[i]; i++)
			printf(" - %15s\n", modules[i]);
		free(modules);
	}
	else 
		retval = irc_send_raw_msg(con, msg);

	free(parse);
	return retval;
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

	conf = config_from_filename("ircb.conf");
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

	module_load_module_dir(&con, conf);

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

	module_unload_all(&con);

	irc_close(&con, "bye.");
	return 0;
}
