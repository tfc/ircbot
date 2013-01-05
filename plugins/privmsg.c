#include <stdio.h>
#include <string.h>

#include "../irc.h"
#include "../plugin.h"

plugin_answer plugin_message_handler(irc_connection *con, irc_msg *msg)
{
	if (strcmp(msg->command, "PRIVMSG")) return (plugin_answer){NULL, 0};

	char *answer;
	asprintf(&answer, "PRIVMSG %s :%s\n", msg->target, "blam.");

	printf("[%s] %s: %s\n", msg->target, msg->source, msg->params);

	return (plugin_answer){answer, 1};
}
