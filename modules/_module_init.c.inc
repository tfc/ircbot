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

void init_module()
{
	void* self_process = dlopen(NULL, 0);
	assert(self_process);

	_ssf = dlsym(self_process, "send_string");
	assert(_ssf);
}

