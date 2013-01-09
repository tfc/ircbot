#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "helpers.h"
#include "config.h"

config* config_from_filename(char *filename)
{
	config *retconf;
	GError *error = NULL;
	gsize group_count;
	unsigned i, n;

	GKeyFile *keyfile = g_key_file_new();

	/* Load the GKeyFile from keyfile.conf or return. */
	if (!g_key_file_load_from_file (keyfile, filename, 0, &error)) {
		printf("%s\n", error->message);
		return NULL;
	}

	gchar **groups = g_key_file_get_groups(keyfile, &group_count);

	retconf = malloc(sizeof(config));

	retconf->group_count = group_count;
	retconf->groups = malloc(group_count * sizeof(config_group));

	for (i=0; i < group_count; i++) {
		config_group *group = &retconf->groups[i];
		group->name = strdup(groups[i]);

		gsize key_count;
		gchar **keys = g_key_file_get_keys(keyfile, groups[i], &key_count, &error);
		group->pair_count = key_count;
		group->pairs = malloc(key_count * sizeof(config_keyvalue));

		for (n=0; n < key_count; n++) {
			config_keyvalue *pair = &group->pairs[n];
			gchar *value = g_key_file_get_string(keyfile, groups[i], keys[n], &error);

			pair->key = strdup(keys[n]);
			pair->value = strdup(value);

			g_free(value);
		}
		g_strfreev(keys);
	}
	g_strfreev(groups);

	g_key_file_free(keyfile);

	return retconf;
}

void config_free(config *conf)
{
	int i, n;

	for (i=0; i < conf->group_count; i++) {
		config_group *g = &conf->groups[i];

		for (n=0; n < g->pair_count; n++) {
			config_keyvalue *p = &g->pairs[n];
			Free_list(p->key, p->value);
		}
		free(g->pairs);
	}
	Free_list(conf->groups, conf);
}

void config_dump(config *conf)
{
	int i, n;

	for (i=0; i < conf->group_count; i++) {
		config_group *g = &conf->groups[i];

		printf("-- [%s]\n", g->name);
		for (n=0; n < g->pair_count; n++) {
			config_keyvalue *p = &g->pairs[n];
			printf("    |-- %15s = %s\n", p->key, p->value);
		}
	}

}

config_group* config_get_group(config *conf, char *groupname)
{
	int i;
	for (i=0; i < conf->group_count; i++)
		if (!strcmp(groupname, conf->groups[i].name))
			return &conf->groups[i];
	return NULL;

}

char *config_get_value(config_group *group, char *key)
{
	int n;
	for (n=0; n < group->pair_count; n++)
		if (!strcmp(key, group->pairs[n].key))
			return group->pairs[n].value;
	return NULL;
}
