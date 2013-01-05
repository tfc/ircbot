#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"
#include "irc.h"
#include "plugin.h"

typedef struct plugin_listitem {
	struct plugin_listitem *next;
	plugin_irc_message_handler handle_msg;
} plugin_listitem;

int plugins_handle_msg(irc_connection *con, irc_msg *msg)
{
	plugin_listitem *p = con->plugins;
	int count = 0;

	while (p) {
		if (p->handle_msg(con, msg)) count++;	
		p = p->next;
	}

	return count;
}

int plugin_add(irc_connection *con, plugin_irc_message_handler handle_msg)
{
	plugin_listitem *p = malloc(sizeof(plugin_listitem));
	if (!p) return -1;

	p->next = NULL;
	p->handle_msg = handle_msg;

	plugin_listitem *l = con->plugins;
	if (l) {
		while (l->next) l = l->next;
		l->next = p;
	}
	else
		con->plugins = p;


	return 0;
}
