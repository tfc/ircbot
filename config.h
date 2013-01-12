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

config* config_from_filename(const char *filename);
void config_dump(config *conf);
void config_free(config *conf);

config_group* config_get_group(config *conf, const char *groupname);
char* config_get_value(config_group *group, const char *key);

/* Helper macro for getting strings from config objects.
 * You really __have to__ define a pointer "g" 
 * of type config_group* to the configuration group
 * you are referring to.
 */
#define Conf(__key, __defaultval) ({ \
		char *s = config_get_value(g, (__key)); \
		s ? s : (__defaultval); \
})

#endif /* __CONFIG_H__ */
