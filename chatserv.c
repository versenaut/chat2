/*
 * 
*/

#include <stdio.h>
#include <stdlib.h>

#include "verse.h"

#include "channel.h"

typedef struct {
	const char	*ip;

	VNodeID		avatar;
	Channel		*chan_default;

	uint16		group, say;
} MainInfo;

/*------------------------------------------------------------------------------------------------ */

static int handle_command(MainInfo *min, const char *channel, User *speaker, const char *text)
{
	if(strcmp(text, "VerseListCommands") == 0)
	{
		user_hear(speaker, "", "server", "/VerseListCommands, /nick <nick>\n");
	}
	else if(strncmp(text, "nick ", 5) == 0)
	{
		printf("wanna be '%s'?\n", text + 5);
	}
	else
	{
		printf("Unknown command '%s'\n", text);
		return 0;
	}
	return 1;
}

static void handle_hear(MainInfo *min, const char *channel, VNodeID sender, const char *text)
{
	size_t	i;
	User	*speaker;
	User	*user;

	if((speaker = user_verse_lookup(sender)) == NULL)
		printf("bad sender\n");

	if(text[0] == '/')	/* If it starts with a slash, it *might* be a command. Make sure. */
	{
		if(handle_command(min, channel, speaker, text + 1))
			return;
	}

	printf("got \"%s\" in channel \"%s\" [%u users]\n", channel, text, user_count());
	for(i = 0; (user = user_index(i)) != NULL; i++)
		user_hear(user, channel, user_get_name(speaker), text);
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
				printf("server say is %u.%u.%u\n", node_id, group_id, method_id);
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
		char	buf[128];

		printf("creating a new user-client for node %u\n", node_id);
		snprintf(buf, sizeof buf, "client-%u", node_id);
		user_verse_new(buf, node_id, group_id, method_id);
	}
	else
		printf("method %u.%u.%u %s() is no good\n", node_id, group_id, method_id, name);
}

static void cb_o_method_group_create(void *user, VNodeID node_id, uint16 group_id, const char *name)
{
	MainInfo	*min = user;

	printf("node %u has method group %u, '%s'\n", node_id, group_id, name);
	if(node_id == min->avatar)
	{
		printf("that's me ...\n");
		if(strcmp(name, "chat_text") == 0 && min->group == (uint16) ~0u)
		{
			VNOParamType	say_ptype[] = { VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING };
			const char	*say_pname[] = { "channel", "text" };

			min->group = group_id;
			verse_send_o_method_group_subscribe(node_id, group_id);
			printf(" my group is %u\n", min->group);
			verse_send_o_method_create(min->avatar, min->group, (uint16) ~0u, "say", sizeof say_pname / sizeof *say_pname, say_ptype, say_pname);
		}
	}
	else if(strcmp(name, "chat_text") == 0)
	{
		printf(" subscribed\n");
		verse_send_o_method_group_subscribe(node_id, group_id);
	}
}

static void cb_node_name_set(void *user, VNodeID node_id, const char *name)
{
	printf("(node %u is named '%s')\n", node_id, name);
}

static void cb_node_create(void *user, VNodeID node_id, VNodeType type, VNodeOwner owner)
{
	MainInfo	*min = user;

	printf("got node %u\n", node_id);
	verse_send_node_subscribe(node_id);

	if(node_id == min->avatar)
	{
		verse_send_o_method_group_create(node_id, (uint16) ~0u, "chat_text");
	}
}

static void cb_connect_accept(void *user, VNodeID avatar, const char *address, const uint8 *host)
{
	MainInfo	*min = user;

	printf("connected as %u to '%s'\n", avatar, address);
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
	user_init();

	min.chan_default = channel_new("");

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
