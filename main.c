#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "irc.h"

int keyboard_input(irc_connection *con)
{
	char msg[512];
	char *ret = fgets(msg, 511, stdin);
	if (!ret) return -1;

	if (!strcmp(msg, "q\n")) return -2;

	return irc_send_raw_msg(con, msg);
}

int main(int argc, char *argv[])
{
	int err;
	irc_connection con;

	err = irc_connect(&con, argv[1], atoi(argv[2]));
	if (err) {
		fprintf(stderr, "Got no connection.");
		exit(1);
	}

	irc_set_nick(&con, "cbot");
	irc_set_user(&con, "cbot_user", "cbot_host", "cbot_servername", "CBot Real Name");

	do {
		err = wait_fill_buffer(&con);
		int msgs = irc_messages_pending(&con);

		while (msgs--) {
			irc_msg *msg = irc_next_message(&con);
			if (!msg) { 
				printf("Erroneous message.\n"); 
				continue; 
			}
			printf("<-- %s {SRC [%s] CMD [%s] TGT [%s]}\n",
				msg->params, msg->source, msg->command, msg->target);
			irc_free_msg(msg);
		}
	} while (keyboard_input(&con) != -2 && err);

	irc_close(&con, "bye.");
}
