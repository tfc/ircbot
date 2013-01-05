#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>

#include "../irc_common_struct.h"

typedef int (*send_string_func)(irc_connection *con, char *buf);
static send_string_func _ssf = NULL;

#define Irc_send(con, ...) do { \
	char *sendmsg; \
	asprintf(&sendmsg, __VA_ARGS__); \
	_ssf(con, sendmsg); \
	free(sendmsg); \
} while(0)

void init_module_generic(send_string_func ssf)
{
    _ssf = ssf;
}

