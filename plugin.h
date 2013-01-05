#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "irc.h"

typedef struct plugin_answer {
	char *response;
	int retval;
} plugin_answer;

typedef plugin_answer (*plugin_irc_message_handler)(irc_connection *con, irc_msg *msg);

int plugins_handle_msg(irc_connection *con, irc_msg *msg);

int plugin_load_plugin_dir(irc_connection *con);

int plugin_add(irc_connection *con, const char *plugin_file);

#endif /* __PLUGIN_H__ */

