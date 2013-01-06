#include <stdio.h>

/*
 * This is an empty example module.
 * It does nothing.
 *
 * To start your own module, do "cp null.c my_module.c" and implement
 * some code within the empty interface functions. :-)
 */

#include "module.h"
#include "_module_init.c.inc"

int module_message_handler(irc_connection *con, irc_msg *msg)
{
	/* Modules which ignore messages, should return 0 */
	return 0;
}

int module_init(irc_connection *con)
{
	/* Do your initialization here. If an error occurs, return something != 0.
	 * The bot will then abort the loading process of this module.
	 */
	return 0;
}

void module_close(void) 
{
	/* Cleanup at unload */
}
