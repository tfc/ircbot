#ifndef __MODULE_SUPPORT_H__
#define __MODULE_SUPPORT_H__

#include "irc.h"

/* Call this to make the list of modules you have loaded
 * handle an IRC message you got from the server.
 */
int module_handle_msg(irc_connection *con, irc_msg *msg);

/* Load all modules for the given IRC connection.
 * What it does right now: look into ./modules/ and
 * load every module object file from there.
 */
int module_load_module_dir(irc_connection *con, config *conf);

/* Load a module providing its module name (e.g. "ctcp")
 */
int module_load(irc_connection *con, config *conf, const char *module_name);

/* Unload a module providing its module name. (e.g. "ctcp")
 */
int module_unload(irc_connection *con, const char *module_name);

/* Unload all modules of a given connection.
 */
void module_unload_all(irc_connection *con);

#endif /* __MODULE_SUPPORT_H__ */

