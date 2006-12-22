/*
 * Command handling.
 *
 * See chatserv.c for legal details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat2.h"

#include "channel.h"
#include "qsarr.h"
#include "user.h"

#include "command.h"
#include "cmd_chanop.h"
#include "cmd_nick.h"
#include "cmd_version.h"

struct Command {
	char	name[32];
	char	label[64];
	char	args[64];
	char	desc[256];

	int	(*handler)(Channel *channel, User *speaker, const char *text);
};

static struct {
	QSArr	*commands;
} CmdInfo;

/*------------------------------------------------------------------------------------------------ */

static int cmp_cmd_sort(const void **ea, const void **eb)
{
	const Command	*ca = *(Command **) ea, *cb = *(Command **) eb;

	return strcmp(ca->name, cb->name);
}

static int cmp_cmd_key(const void *element, const void *key)
{
	return strcmp(((Command *) element)->name, key);
}

static int verse_list_commands(Channel *channel, User *speaker, const char *text)
{
	command_send_list(speaker);

	return 1;
}

void command_init(void)
{
	CmdInfo.commands = qsarr_new(cmp_cmd_sort, cmp_cmd_key);

	command_new("/VerseListCommands", "List available commands", "none", "List available commands, one per response.", verse_list_commands);
	command_new("/nick", "Set nickname", "string(name)", "Set a new nickname.", cmd_nick);
	command_new("/join", "Join channel", "channel(channel)", "Join a channel, which is created if it does not exist.", cmd_join);
	command_new("/leave", "Leave channel", "channel(channel)", "Leave a channel, which is destroyed if it becomes empty.", cmd_leave);
	command_new("/listchan", "List channels", "none", "List available channels, by name.", cmd_listchan);
	command_new("/topic", "Set channel topic", "string(topic)", "Set the topic of a channel, a description about what is currently going on.", cmd_topic);
	command_new("/version", "Check server version", "none", "Check the version of the chat server.", cmd_version);
	command_new("/who", "List users in channel", "none", "List the users in the addressed channel.", cmd_who);
}

Command * command_new(const char *name, const char *label,
		      const char *args, const char *description,
		      int (*handler)(Channel *channel, User *speaker, const char *text))
{
	Command	*c;

	if((c = command_lookup(name)) != NULL)
	{
		fprintf(stderr, "Can't register command \"%s\", already exists\n", name);
		return NULL;
	}
	if((c = malloc(sizeof *c)) != NULL)
	{
		snprintf(c->name, sizeof c->name, "%s", name);
		snprintf(c->label, sizeof c->label, "%s", label);
		snprintf(c->args, sizeof c->args, "%s", args);
		snprintf(c->desc, sizeof c->desc, "%s", description);
		c->handler = handler;
		qsarr_insert(CmdInfo.commands, c);
	}
	return c;
}

Command * command_lookup(const char *name)
{
	return qsarr_lookup(CmdInfo.commands, name);
}

int command_run(Command *cmd, Channel *channel, User *speaker, const char *text)
{
	if(cmd == NULL || channel == NULL || speaker == NULL || text == NULL)
		return 0;
	return cmd->handler(channel, speaker, text);
}

/* Format a command for list output. */
static const char * list_format(const Command *cmd)
{
	static char	buf[3072];

	if(snprintf(buf, sizeof buf, "/VerseListCommands %s \"%s\" %s \"%s\"\n",
		   cmd->name, cmd->label, cmd->args, cmd->desc) < sizeof buf)
	{
		return buf;
	}
	return NULL;
}

/* Send list of registered commands to the given user. */
void command_send_list(User *user)
{
	int	i;
	Command	*cmd;

	if(user == NULL)
		return;
	for(i = 0; (cmd = qsarr_index(CmdInfo.commands, i)) != NULL; i++)
	{
		const char	*list = list_format(cmd);
		if(list != NULL)
			user_hear(user, "", NULL, list);
		else
			fprintf(stderr, "Can't list command '%s', too long\n", cmd->name);
	}
}
