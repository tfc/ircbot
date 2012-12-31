#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "irc.h"

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
			char *msg = irc_next_message(&con);
			printf("<-- %s\n", msg);
			free(msg);
		}
	} while (err);

	irc_close(&con, "bye.");
}
