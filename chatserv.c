/*
 * A Verse chat v2 protocol implementation.
 * 
 * Copyright (c) 2006 PDC, KTH. Written by Emil Brink.
 * 
 * Released under a FreeBSD license.
*/

#include <stdio.h>
#include <stdlib.h>

#include "verse.h"

#include "channel.h"
#include "command.h"
#include "nodedb.h"
#include "user_verse.h"

#if defined _WIN32
#define	snprintf	_snprintf
#endif

typedef struct {
	const char	*ip;

	VNodeID		avatar;

	uint16		group, say;
} MainInfo;

/*------------------------------------------------------------------------------------------------ */

/* Try to run the <text> as a command. Returns 1 if the text boiled down to a known command, 0 if not. */
static int handle_command(const char *channel, User *speaker, const char *text)
{
	char	cmd[128], args[512], *put;
	size_t	len;
	Command	*c;

	/* Strip out first word, which is the command name. */
	for(put = cmd; !isspace(*text) && put - cmd < sizeof cmd - 1;)
		*put++ = *text++;
	if(!isspace(*text))	/* Command very long? */
		return 0;
	*put = '\0';
	while(isspace(*text))
		text++;
	/* Split out the arguments too, and chop off trailing whitespace. */
	len = snprintf(args, sizeof args, "%s", text);
	for(; len > 0 && isspace(args[--len]);)
		args[len] = '\0';
	if((c = command_lookup(cmd)) != NULL)
	{
		command_run(c, channel_lookup(channel), speaker, args);
		return 1;
	}
	return 0;
}

static void handle_hear(MainInfo *min, const char *channel, VNodeID sender, const char *text)
{
	size_t	i;
	Channel	*ch;
	User	*speaker;
	User	*user;

	if((ch = channel_lookup(channel)) == NULL)
	{
		fprintf(stderr, "Got hear() in unknown channel \"%s\", ignoring\n", channel);
		return;
	}

	if((speaker = user_verse_from_node_id(sender)) == NULL)
	{
		fprintf(stderr, "Got hear() from unknown user (node %u), ignoring\n", sender);
		return;
	}

	if(!channel_user_is_member(ch, speaker))
	{
		fprintf(stderr, "Got hear() from non-channel member, igoring\n");
		return;
	}

	if(text[0] == '/')	/* If it starts with a slash, it *might* be a command. Make sure. */
	{
		if(handle_command(channel, speaker, text))
			return;	/* If command was recognized, we're done. */
	}
	channel_hear(ch, speaker, text);
}

static void cb_o_method_call(void *user, VNodeID node_id, uint16 group_id, uint16 method_id, VNodeID sender, const VNOPackedParams *params)
{
	MainInfo	*min = user;

	if(node_id == min->avatar &&
	   group_id == min->group &&
	   method_id == min->say)
	{
		VNOParamType	type[2] = { VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING };
		VNOParam	value[2];

		if(verse_method_call_unpack(params, sizeof type / sizeof *type, type, value))
			handle_hear(min, value[0].vstring, sender, value[1].vstring);
	}
}

static void cb_o_method_create(void *user, VNodeID node_id, uint16 group_id, uint16 method_id, const char *name, uint8 param_count, const VNOParamType *param_types, const char * * param_names)
{
	MainInfo	*min = user;

	if(node_id == min->avatar)
	{
		/* Look for the official "say" method. */
		if(strcmp(name, "say") == 0 &&
		   param_count == 2 &&
		   param_types[0] == VN_O_METHOD_PTYPE_STRING &&
		   param_types[1] == VN_O_METHOD_PTYPE_STRING &&
		   strcmp(param_names[0], "channel") == 0 &&
		   strcmp(param_names[1], "text") == 0)
		{
			if(min->say == (uint16) ~0u)
			{
				min->say = method_id;
				printf("Server registered, ready to receive data\n");
			}
		}
		return;
	}

	/* Look for the official "hear" method, very closely. */
	if(strcmp(name, "hear") == 0 &&
	   param_count == 3 &&
	   param_types[0] == VN_O_METHOD_PTYPE_STRING &&
	   param_types[1] == VN_O_METHOD_PTYPE_STRING &&
	   param_types[2] == VN_O_METHOD_PTYPE_STRING &&
	   strcmp(param_names[0], "channel") == 0 &&
	   strcmp(param_names[1], "speaker") == 0 &&
	   strcmp(param_names[2], "text") == 0)
	{
		Node	*n;

		if((n = nodedb_lookup(node_id)) != NULL)
		{
			User	*u = user_verse_new(nodedb_get_name(n), node_id, group_id, method_id);
			channel_user_add(channel_default(), u);
			printf("User \"%s\" (node %u) recognized\n", nodedb_get_name(n), node_id);
		}
	}
	else
		printf("method %u.%u.%u %s() is no good\n", node_id, group_id, method_id, name);
}

static void cb_o_method_group_create(void *user, VNodeID node_id, uint16 group_id, const char *name)
{
	MainInfo	*min = user;

	if(node_id == min->avatar)
	{
		if(strcmp(name, "chat_text") == 0 && min->group == (uint16) ~0u)
		{
			VNOParamType	say_ptype[] = { VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING };
			const char	*say_pname[] = { "channel", "text" };

			min->group = group_id;
			verse_send_o_method_group_subscribe(node_id, group_id);
			verse_send_o_method_create(min->avatar, min->group, (uint16) ~0u, "say", sizeof say_pname / sizeof *say_pname, say_ptype, say_pname);
		}
	}
	else if(strcmp(name, "chat_text") == 0)
		verse_send_o_method_group_subscribe(node_id, group_id);
}

static void cb_node_name_set(void *user, VNodeID node_id, const char *name)
{
	Node	*n;

	if((n = nodedb_lookup(node_id)) != NULL)
		nodedb_set_name(n, name);
}

static void cb_node_create(void *user, VNodeID node_id, VNodeType type, VNodeOwner owner)
{
	MainInfo	*min = user;

	nodedb_new(node_id);
	verse_send_node_subscribe(node_id);

	if(node_id == min->avatar)
		verse_send_o_method_group_create(node_id, (uint16) ~0u, "chat_text");
}

static void cb_connect_accept(void *user, VNodeID avatar, const char *address, const uint8 *host)
{
	MainInfo	*min = user;

	printf("Connected as %u to %s\n", avatar, address);
	min->avatar = avatar;
	verse_send_node_index_subscribe(1 << V_NT_OBJECT);
	verse_send_node_name_set(avatar, "chatserv");
}

int main(int argc, char *argv[])
{
	MainInfo	min;
	int		i;

	min.ip = "localhost";
	min.group = ~0;
	min.say = ~0;

	for(i = 1; argv[i] != NULL; i++)
	{
		if(strncmp(argv[i], "-ip=", 4) == 0)
			min.ip = argv[i] + 4;
		else
			fprintf(stderr, "chatserv: Ignoring argument \"%s\"\n", argv[i]);
	}

	channel_init();
	command_init();
	nodedb_init();
	user_init();

	verse_callback_set((void *) verse_send_connect_accept,		(void *) cb_connect_accept,		&min);
	verse_callback_set((void *) verse_send_node_create,		(void *) cb_node_create,		&min);
	verse_callback_set((void *) verse_send_node_name_set,		(void *) cb_node_name_set,		&min);
	verse_callback_set((void *) verse_send_o_method_group_create,	(void *) cb_o_method_group_create,	&min);
	verse_callback_set((void *) verse_send_o_method_create,		(void *) cb_o_method_create,		&min);
	verse_callback_set((void *) verse_send_o_method_call,		(void *) cb_o_method_call,		&min);

	verse_send_connect("chatserv", "<secret>", min.ip, NULL);

	while(1)
	{
		verse_callback_update(10000);
	}

	return EXIT_SUCCESS;
}
