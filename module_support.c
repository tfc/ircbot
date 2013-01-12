#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>

#include "helpers.h"
#include "irc.h"
#include "config.h"
#include "module_support.h"
#include "modules/module.h"

#define MODULE_PATH "modules"

#ifdef __APPLE__
#define MODULE_SUFFIX ".dylib"
#else
#define MODULE_SUFFIX ".so"
#endif

#define LINK_FUNC(__funcsym, __funcname, __failret) do { \
	__funcsym = dlsym(module_libfile, __funcname); \
	if (!__funcsym) return __failret; \
} while (0)

#define Module_path(__module_name) ({ \
	char *tmp; \
	asprintf(&tmp, "%s/%s%s", MODULE_PATH, __module_name, MODULE_SUFFIX); \
	tmp; \
})

#define Module_name(__module_path) ({ \
	char *copy = strdup(__module_path); \
	char *tmp = basename(copy); \
	char *modname = strdup(strtok(tmp, ".")); \
	free(copy); \
	modname; \
})

/* Every module has actually two initialization procedures.
 * - One implemented by the module programmer for his private stuff.
 * - One implemented by us initializing function pointers since
 *   this is all linked at runtime.
 * This typedef is for the latter.
 */
typedef void (*init_module_func)();

/* We will be just keeping a linked list of loaded modules.
 * Every item keeps pointers to the modules' functions/procedures
 */
typedef struct module_listitem {
	char *name;
	struct module_listitem *next;

	void *libhandle;
	module_init init;
	module_message_handler handle_msg;
	module_close close;
} module_listitem;

static module_listitem* module_by_name(irc_connection *con, const char *name, module_listitem **predecessor)
{
	module_listitem *l = con->modules;
	module_listitem *l2;

	while (l && strcmp(l->name, name)) {
		l2 = l;
		l = l->next;
	}

	if (predecessor) *predecessor = l2;
	return l;
}

/* 
 * The bot code will call this function on every IRC message
 * and the modules will dispatch them.
 */
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

static int module_add(irc_connection *con, config_group *group, const char *module_file)
{
	init_module_func mod_generic_init;
	module_message_handler mod_msg_handler;
	module_init mod_init;
	module_close mod_close;

	if (module_by_name(con, Module_name(module_file), NULL)) return -9;

	void* module_libfile = dlopen(module_file, RTLD_NOW | RTLD_LOCAL);
	if (!module_libfile) {
		Printerr("dlopen: %s\n", dlerror());
		return -1;
	}

	module_message_handler module_msg_handler = dlsym(module_libfile, "module_message_handler");
	if (!module_msg_handler) return -2;

	LINK_FUNC(mod_generic_init, "init_module_generic", -3);
	mod_generic_init(irc_send_raw_msg, config_get_value, group);
	LINK_FUNC(mod_init, "module_init", -4);
	LINK_FUNC(mod_msg_handler, "module_message_handler", -5);
	LINK_FUNC(mod_close, "module_close", -6);

	if (mod_init(con)) return -7;

	module_listitem *p = malloc(sizeof(module_listitem));
	if (!p) return -8;

	/* Ok, everything seems to be fine. So we can finally populate the
	 * module structure's pointers and add it to the list of 
	 * loaded modules! */

	p->name = Module_name(module_file);
	p->next = NULL;
	p->libhandle = module_libfile;
	p->init = mod_init;
	p->handle_msg = module_msg_handler;
	p->close = mod_close;

	module_listitem *l = con->modules;
	if (l) {
		while (l->next) l = l->next;
		l->next = p;
	}
	else
		con->modules = p;

	return 0;
}

int module_load(irc_connection *con, config *conf, const char *module_name)
{
	int ret;
	char *module_path = Module_path(module_name);
	ret = module_add(con, config_get_group(conf, module_name), module_path);

	free(module_path);
	return ret;
}

static void free_module(module_listitem *m)
{
	assert(m);
	m->close();
	dlclose(m->libhandle);
	Free_list(m->name, m);
}

int module_unload(irc_connection *con, const char *module_name)
{
	int ret = 0;

	module_listitem *l = con->modules;
	module_listitem *l2;

	l = module_by_name(con, module_name, &l2);

	if (l == NULL) return -9; // Not found!

	// Remove item from list
	if (l2 == NULL) con->modules = l->next;
	else 		l2->next = l->next;

	free_module(l);

	return ret;
}

int module_load_module_dir(irc_connection *con, config *conf)
{
	int ret;
	DIR *module_dir = opendir(MODULE_PATH);
	if (!module_dir) return -1;

	int modules_loaded = 0;
	struct dirent *entry;

	/* search through the module directory and
	 * load everything what looks like a module file.
	 */

	while ((entry = readdir(module_dir))) {
		char *filename = entry->d_name;
		if (filename[0] == '.') continue;

		char *dstr = strstr(filename, MODULE_SUFFIX);
		if (!dstr) continue;

		char *module_path;
		asprintf(&module_path, "%s/%s", MODULE_PATH, filename);

		char *modname = Module_name(filename);
		assert(modname);

		printf("Loading module %15s ... ", modname);
		ret = module_add(con, config_get_group(conf, modname), module_path);
		if (!ret) {
			modules_loaded++;
			printf("OK!\n");
		}
		else 
			printf("Error! (%d)\n", ret);

		Free_list(module_path, modname);
	}

	return modules_loaded;
}

void module_unload_all(irc_connection *con)
{
	module_listitem *m = con->modules;

	while (m) {
		module_listitem *next = m->next;
		printf("Unloading module %15s\n", m->name);
		free_module(m);
		m = next;
	}

	con->modules = NULL;
}
