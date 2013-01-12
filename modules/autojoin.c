#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "module.h"
#include "_module_init.c.inc"
#include "../irc_codes.h"
#include "../config.h"
#include "../helpers.h"

static char *channels = NULL;

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	if (msg->command_num != RPL_ENDOFMOTD) return 0;

	Irc_send(con, "JOIN %s\n", channels);

	return 1;
}

int module_init(irc_connection *con)
{
	channels = Conf("channels", "");

	if (!strlen(channels)) return 1;

	return 0;
}

void module_close(void) 
{
}
