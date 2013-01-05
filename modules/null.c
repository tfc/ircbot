#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_module_init.c.inc"

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	return 0;
}

int module_init(irc_connection *con)
{
	return 0;
}

void module_close(void) 
{
}
