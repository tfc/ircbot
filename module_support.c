#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>

#include "helpers.h"
#include "irc.h"
#include "module_support.h"

#ifdef __APPLE__
#define MODULE_SUFFIX ".dylib"
#else
#define MODULE_SUFFIX ".so"
#endif

typedef struct module_listitem {
	struct module_listitem *next;
	module_irc_message_handler handle_msg;
} module_listitem;

int module_handle_msg(irc_connection *con, irc_msg *msg)
{
	module_listitem *p = con->modules;
	int count = 0;

	while (p) {
		if (p->handle_msg(con, msg)) count++;	
		p = p->next;
	}

	return count;
}

static int module_add(irc_connection *con, const char *module_file)
{
	void* module_libfile = dlopen(module_file, RTLD_NOW | RTLD_LOCAL);
	if (!module_libfile) return -1;

	module_irc_message_handler module_msg_handler = dlsym(module_libfile, "module_message_handler");
	if (!module_msg_handler) return -2;

	init_module_func mod_init = dlsym(module_libfile, "init_module");
	if (!mod_init) return -3;

	mod_init(send_string);

	module_listitem *p = malloc(sizeof(module_listitem));
	if (!p) return -3;

	p->next = NULL;
	p->handle_msg = module_msg_handler;

	module_listitem *l = con->modules;
	if (l) {
		while (l->next) l = l->next;
		l->next = p;
	}
	else
		con->modules = p;

	return 0;
}

int module_load_module_dir(irc_connection *con)
{
	int ret;
	DIR *module_dir = opendir("./modules");
	if (!module_dir) return -1;

	int modules_loaded = 0;
	struct dirent *entry;

	while ((entry = readdir(module_dir))) {
		char *filename = entry->d_name;
		if (filename[0] == '.') continue;

		char *dstr = strstr(filename, MODULE_SUFFIX);
		if (!dstr) continue;

		char *plug_path;
		asprintf(&plug_path, "./modules/%s", filename);

		printf("Loading module \"%s\"...\t", filename);
		ret = module_add(con, plug_path);
		if (!ret) {
			modules_loaded++;
			printf("OK!\n");
		}
		else 
			printf("Error! (%d)\n", ret);

		free(plug_path);
	}

	return modules_loaded;
}
