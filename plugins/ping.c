#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../irc.h"
#include "../plugin.h"

plugin_answer plugin_message_handler(irc_connection *con, irc_msg *msg)
{
	if (strcmp(msg->command, "PING")) return (plugin_answer){NULL, 0};
	char *answer;
	asprintf(&answer, "%s\n", msg->raw_str);
	answer[1] = 'O';

	printf("Got a PING signal (%s).\n", msg->raw_str);

	return (plugin_answer){answer, 1};
}
