#include <stdio.h>
#include <string.h>

#include "_module_init.c.inc"

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	if (strcmp(msg->command, "PRIVMSG")) return 0;

	Irc_send(con, "PRIVMSG %s :%s\n", msg->target, "blam.");

	printf("[%s] %s: %s\n", msg->target, msg->source, msg->params);

	return 1;
}