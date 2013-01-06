#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>

#include "../irc_common_struct.h"

typedef int (*send_string_func)(irc_connection *con, char *buf);
static send_string_func _ssf = NULL;

void init_module_generic(send_string_func ssf)
{
    _ssf = ssf;
}

