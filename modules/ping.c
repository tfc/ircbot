#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "module.h"
#include "_module_init.c.inc"

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	if (strcmp(msg->command, "PING")) return 0;

	/* PING messages look like "PING *_magic_number_*"
	 * and the only correct response is "PONG *_magic_number_*"
	 * so we copy the string and change the "PING" to a "PONG".
	 */
	char *answer = strdup(msg->raw_str);
	answer[1] = 'O';

	/* Don't forget to append a \n to the string. */
	Irc_send(con, "%s\n", answer);
	free(answer);

#if 0
	printf("Got a PING signal (%s).\n", msg->raw_str);
#endif

	return 1;
}

int module_init(irc_connection *con)
{
	return 0;
}

void module_close(void) 
{

}

