#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>

#include "helpers.h"
#include "irc.h"
#include "plugin.h"

#ifdef __APPLE__
#define PLUGIN_SUFFIX ".dylib"
#else
#define PLUGIN_SUFFIX ".so"
#endif

typedef struct plugin_listitem {
	struct plugin_listitem *next;
	plugin_irc_message_handler handle_msg;
} plugin_listitem;

int plugins_handle_msg(irc_connection *con, irc_msg *msg)
{
	plugin_listitem *p = con->plugins;
	int count = 0;

	while (p) {
		plugin_answer answ;
		answ = p->handle_msg(con, msg);
		if (answ.retval) {
			count++;	
			if (answ.response)
				irc_send_raw_msg(con, answ.response);
		}
		p = p->next;
	}

	return count;
}

int plugin_add(irc_connection *con, const char *plugin_file)
{
	void* plugin_libfile = dlopen(plugin_file, RTLD_LAZY);
	if (!plugin_libfile) return -1;

	plugin_irc_message_handler plugin_msg_handler = dlsym(plugin_libfile, "plugin_message_handler");
	if (!plugin_msg_handler) return -2;

	plugin_listitem *p = malloc(sizeof(plugin_listitem));
	if (!p) return -3;

	p->next = NULL;
	p->handle_msg = plugin_msg_handler;

	plugin_listitem *l = con->plugins;
	if (l) {
		while (l->next) l = l->next;
		l->next = p;
	}
	else
		con->plugins = p;

	return 0;
}

int plugin_load_plugin_dir(irc_connection *con)
{
	DIR *plugin_dir = opendir("./plugins");
	if (!plugin_dir) return -1;

	int plugins_loaded = 0;
	struct dirent *entry;

	while ((entry = readdir(plugin_dir))) {
		char *filename = entry->d_name;
		if (filename[0] == '.') continue;

		char *dstr = strstr(filename, PLUGIN_SUFFIX);
		if (!dstr) continue;

		char *plug_path;
		asprintf(&plug_path, "./plugins/%s", filename);

		printf("Loading plugin \"%s\"...\t", filename);
		if (!plugin_add(con, plug_path)) {
			plugins_loaded++;
			printf("OK!\n");
		}
		else 
			printf("Error!\n");

		free(plug_path);
	}

	return plugins_loaded;
}
