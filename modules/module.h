#ifndef __MODULE_H__
#define __MODULE_H__

#ifdef __MODULE_SUPPORT_H__
typedef int (*module_message_handler)(irc_connection *con, irc_msg *msg);
typedef int (*module_init)(irc_connection *con);
typedef void (*module_close)(void);
#else
int module_message_handler(irc_connection *con, irc_msg *msg);
int module_init(irc_connection *con);
void module_close(void);
#endif

#endif /* __MODULE_H__ */
