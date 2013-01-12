#include <stdio.h>
#include <string.h>

#include "module.h"
#include "_module_init.c.inc"
#include "../irc_codes.h"

#define Is_cmd(__cmd) (!strcmp(msg->command, __cmd))

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	int ret = 1;
	int skip = 1;
	switch (msg->command_num) {
		case RPL_MOTDSTART:
			printf("Message of the day:\n");
			break;
		case RPL_MOTD:
			printf("%s\n", msg->params);
			break;
		case RPL_TOPIC:
			printf("%s channel topic: %s\n", msg->target, msg->params);
			break;
		case RPL_NOTOPIC:
			printf("%s channel topic: [none]\n", msg->target);
			break;
		case RPL_NAMREPLY:
			printf("%s users: %s\n", msg->target, msg->params);
			break;
		case RPL_ENDOFNAMES:
		case 333:
			break;
		case 0:
		default:
			skip = 0;
	}

	if (skip) return ret;

	if (Is_cmd("PRIVMSG")) {
		printf("[%s] %s: %s\n", msg->target, msg->src_nick, msg->params);
	}
	else if (Is_cmd("JOIN")) {
		printf("Joined channel %s\n", msg->params);
	}
	else
		ret = 0;

	return ret;
}

int module_init(irc_connection *con)
{
	return 0;
}

void module_close(void) 
{
}

