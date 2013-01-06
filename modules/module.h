#ifndef __MODULE_H__
#define __MODULE_H__

#ifdef __MODULE_SUPPORT_H__
typedef int (*module_message_handler)(irc_connection *con, irc_msg *msg);
typedef int (*module_init)(irc_connection *con);
typedef void (*module_close)(void);
#else

#include "../irc_codes.h"
#include "../irc_common_struct.h"

/* As a module programmer you need to implement the following 3 functions: 
 * --> see null.c or the other modules
 */

/* This function is called on every IRC message which is to be handled
 * by modules.
 * Let your code decide if you ignore the message and return 0 in this case
 * or process it and return 1.
 */
int module_message_handler(irc_connection *con, irc_msg *msg);

/*
 * Init your data like regular expressions etc. here. 
 * This is called when your module gets loaded.
 */
int module_init(irc_connection *con);

/*
 * Free your data here.
 * This is called when your module gets unloaded.
 */
void module_close(void);

/* 
 * If your module sends IRC messages back to the server,
 * use this macro.
 *
 * Irc_send(con, "My message nr %d %s\n", x, "with an additional string.");
 *
 * Don't forget to append a newline to every message.
 * Sending multiple messages can be done within one string this way.
 */
#define Irc_send(con, ...) do { \
	char *sendmsg; \
	asprintf(&sendmsg, __VA_ARGS__); \
	_ssf(con, sendmsg); \
	free(sendmsg); \
} while(0)

#endif

#endif /* __MODULE_H__ */
