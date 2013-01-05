#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_module_init.c.inc"

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	if (strcmp(msg->command, "PING")) return 0;

	char *answer = strdup(msg->raw_str);
	answer[1] = 'O';
	Irc_send(con, "%s\n", answer);
	free(answer);

	printf("Got a PING signal (%s).\n", msg->raw_str);

	return 1;
}
