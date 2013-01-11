#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>

#include "../irc_common_struct.h"
#include "../config.h"

typedef int (*send_string_func)(irc_connection *con, char *buf);
static send_string_func _ssf = NULL;

typedef char* (*config_get_value_func)(config_group *group, char *key);
static config_get_value_func _cgvf = NULL;

static config_group *g;

void init_module_generic(send_string_func ssf, config_get_value_func cgvf, config_group *group)
{
    _ssf = ssf;
    _cgvf = cgvf;
    g = group;
}

#undef Conf
#define Conf(__key, __defaultval) ({ \
		char *s = _cgvf(g, (__key)); \
		s ? s : (__defaultval); \
})
