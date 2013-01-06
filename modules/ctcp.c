#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "_module_init.c.inc"

static GRegex *ping_pattern    = NULL;
static GRegex *version_pattern = NULL;

static int ctcp_ping(irc_connection *con, irc_msg *msg)
{
	gboolean ret;
	GMatchInfo *info;

	ret = g_regex_match(ping_pattern, msg->params, 0, &info);

	int matches = g_match_info_get_match_count(info);
	if (matches != 2) return 0;

	gchar *ping_id_str = g_match_info_fetch(info, 1);
	Irc_send(con, "NOTICE %s :\001PING %s\001\n", msg->src_nick, ping_id_str);
	g_free(ping_id_str);

	return 1;
}

static int ctcp_version(irc_connection *con, irc_msg *msg)
{
	gboolean ret;
	GMatchInfo *info;

	ret = g_regex_match(version_pattern, msg->params, 0, &info);

	int matches = g_match_info_get_match_count(info);
	if (matches != 1) return 0;

	Irc_send(con, "NOTICE %s :\001VERSION %s\001\n", 
			msg->src_nick, "plain C irc bot: https://github.com/tfc/ircbot");

	return 1;
}
int module_message_handler(irc_connection *con, irc_msg *msg)
{
	if (strcmp(msg->command, "PRIVMSG")) return 0;

	return ctcp_ping(con, msg) || ctcp_version(con, msg);
}

int module_init(irc_connection *con)
{
	ping_pattern = g_regex_new("^[\001]PING (.+)[\001]$", 0, 0, NULL);
	version_pattern = g_regex_new("^[\001]VERSION[\001]$", 0, 0, NULL);

	if (!ping_pattern || !version_pattern) return -1;

	return 0;
}

void module_close(void) 
{
	g_regex_unref(ping_pattern);
	g_regex_unref(version_pattern);
}
