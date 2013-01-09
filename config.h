#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct config_keyvalue {
	char *key;
	char *value;
} config_keyvalue;

typedef struct config_group {
	char *name;
	unsigned pair_count;
	config_keyvalue *pairs;
} config_group;

typedef struct config {
	unsigned group_count;
	config_group *groups;
} config;

config* config_from_filename(char *filename);
void config_dump(config *conf);
void config_free(config *conf);

config_group* config_get_group(config *conf, char *groupname);
char* config_get_value(config_group *group, char *key);

#endif /* __CONFIG_H__ */
