#ifndef __MODULE_SUPPORT_H__
#define __MODULE_SUPPORT_H__

#include "irc.h"

typedef int (*module_irc_message_handler)(irc_connection *con, irc_msg *msg);

int module_handle_msg(irc_connection *con, irc_msg *msg);

int module_load_module_dir(irc_connection *con);

typedef void (*init_module_func)(void);

#endif /* __MODULE_SUPPORT_H__ */

