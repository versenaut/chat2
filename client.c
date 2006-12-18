/*
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "winsock2.h"
#endif

#include "verse.h"

typedef struct {
	const char	*ip;
	VNodeID		avatar;

	VNodeID		server;
	uint16		group, say;

	uint16		my_group, hear;

	int		running;
} MainInfo;

static const char * read_line(void)
{
	static char	line[1024];
	static size_t	len = 0;
	fd_set		in;
	struct timeval	timeout;

	FD_ZERO(&in);
	FD_SET(0, &in);

	timeout.tv_sec  = 0;
	timeout.tv_usec = 10000;

	if(select(1, &in, NULL, NULL, &timeout))
	{
		char	buf[1024];
		int	got, i;

		if((got = read(0, buf, sizeof buf)) > 0)
		{
			for(i = 0; i < got; i++, len++)
			{
				if(len >= sizeof line - 1)
					len = 0;
				line[len] = buf[i];
				if(line[len] == '\n')
				{
					line[len] = '\0';
					len = 0;
					return line;
				}
			}
		}
		else
			return "QUIT";
	}
	return NULL;
}

static void send_text(MainInfo *min, const char *text)
{
	if(min->server != ~0u && text != NULL && text[0] != '\0')
	{
		VNOParamType	type[2];
		VNOParam	value[2];
		VNOPackedParams	*pp;

		type[0] = type[1] = VN_O_METHOD_PTYPE_STRING;
		value[0].vstring = "";
		value[1].vstring = (char *) text;
		if((pp = verse_method_call_pack(sizeof type / sizeof *type, type, value)) != NULL)
			verse_send_o_method_call(min->server, min->group, min->say, 0, pp);
	}
}

static void mainloop(MainInfo *min)
{
	const char	*line;

	while(min->running)
	{
		verse_callback_update(10000);
		if((line = read_line()) != NULL)
		{
			if(strcmp(line, "QUIT") == 0)
				min->running = 0;
			else
				send_text(min, line);
		}
	}
}

static void cb_o_method_call(void *user, VNodeID node_id, uint16 group_id, uint16 method_id, VNodeID sender,
			     const VNOPackedParams *params)
{
	MainInfo	*min = user;

	if(node_id == min->avatar &&
	   group_id == min->my_group &&
	   method_id == min->hear &&
	   sender == min->server)
	{
		VNOParamType	type[] = { VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING };
		VNOParam	value[3];

		if(verse_method_call_unpack(params, 3, type, value))
		{
			printf("[%s] %s: %s\n", value[0].vstring, value[1].vstring, value[2].vstring);
		}
	}
}

static void cb_o_method_create(void *user, VNodeID node_id, uint16 group_id, uint16 method_id, const char *name,
			       uint8 param_count, const VNOParamType *types, const char **names)
{
	MainInfo	*min = user;

	if(node_id != min->avatar &&
	   strcmp(name, "say") == 0 &&
	   param_count == 2 &&
	   types[0] == VN_O_METHOD_PTYPE_STRING &&
	   types[1] == VN_O_METHOD_PTYPE_STRING &&
	   strcmp(names[0], "channel") == 0 &&
	   strcmp(names[1], "text") == 0)
	{
		if(min->server == ~0)
		{
			min->server = node_id;
			min->group = group_id;
			min->say = method_id;
			printf("Found chat server, method %u.%u.%u\n", node_id, group_id, method_id);
		}
	}
	else if(node_id == min->avatar &&
		group_id == min->my_group &&
		min->hear == (uint16) ~0u &&
		strcmp(name, "hear") == 0 &&
		param_count == 3 &&
		types[0] == VN_O_METHOD_PTYPE_STRING &&
		types[1] == VN_O_METHOD_PTYPE_STRING &&
		types[2] == VN_O_METHOD_PTYPE_STRING &&
		strcmp(names[0], "channel") == 0 &&
		strcmp(names[1], "speaker") == 0 &&
		strcmp(names[2], "text") == 0)
	{
		min->my_group = group_id;
		min->hear = method_id;
		printf("my hear is %u.%u.%u\n", node_id, group_id, method_id);
	}
}

static void cb_o_method_group_create(void *user, VNodeID node_id, uint16 group_id, const char *name)
{
	MainInfo	*min = user;

	if(node_id != min->avatar && strcmp(name, "chat_text") == 0)
	{
		verse_send_o_method_group_subscribe(node_id, group_id);
		printf("subscribing to chat group in %u, hunting server\n", node_id);
	}
	else if(node_id == min->avatar && strcmp(name, "chat_text") == 0 && min->my_group == (uint16) ~0u)
	{
		VNOParamType	type[3] = { VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING };
		const char	*name[] = { "channel", "speaker", "text" };

		min->my_group = group_id;
		verse_send_o_method_group_subscribe(node_id, group_id);
		verse_send_o_method_create(min->avatar, min->my_group, (uint16) ~0u, "hear",
					   sizeof type / sizeof *type,
					   type, name);
	}
}

static void cb_node_create(void *user, VNodeID node_id, VNodeType type, VNodeOwner owner)
{
	verse_send_node_subscribe(node_id);
}

static void cb_connect_accept(void *user, VNodeID avatar, const char *address, const uint8 *key)
{
	MainInfo	*min = user;

	min->avatar = avatar;
	printf("Connected as %u to '%s'\n", avatar, address);
	verse_send_node_index_subscribe(1 << V_NT_OBJECT);
	verse_send_o_method_group_create(avatar, (uint16) ~0u, "chat_text");
}

int main(int argc, char *argv[])
{
	MainInfo	min;
	int		i;

	min.ip = "localhost";

	min.avatar = ~0u;

	min.server = ~0u;
	min.group = min.say = (uint16) ~0u;

	min.my_group = min.hear = (uint16) ~0u;

	for(i = 1; argv[i] != NULL; i++)
	{
		if(strncmp(argv[i], "-ip=", 4) == 0)
			min.ip = argv[i] + 4;
		else
			fprintf(stderr, "Ignoring unknown argument \"%s\"\n", argv[i]);
	}
	min.running = 1;

	verse_callback_set((void *) verse_send_connect_accept,		(void *) cb_connect_accept,		&min);
	verse_callback_set((void *) verse_send_node_create,		(void *) cb_node_create,		&min);
	verse_callback_set((void *) verse_send_o_method_group_create,	(void *) cb_o_method_group_create,	&min);
	verse_callback_set((void *) verse_send_o_method_create,		(void *) cb_o_method_create,		&min);
	verse_callback_set((void *) verse_send_o_method_call,		(void *) cb_o_method_call,		&min);
 
	verse_send_connect("chat", "<secret>", min.ip, NULL);

	mainloop(&min);

	return EXIT_SUCCESS;
}
