#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "irc.h"

typedef int (*plugin_irc_message_handler)(irc_connection *con, irc_msg *msg);

int plugins_handle_msg(irc_connection *con, irc_msg *msg);

int plugin_add(irc_connection *con, plugin_irc_message_handler handle_msg);

#endif /* __PLUGIN_H__ */

