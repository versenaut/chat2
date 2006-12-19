/*
 * 
*/

#include <stdio.h>
#include <string.h>

#include "user-verse.h"

static void user_verse_hear(User *user, const char *channel, const char *speaker, const char *text)
{
	UserClient	*uc = (UserClient *) user;
	VNOPackedParams	*pp;
	VNOParamType	type[] = { VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING, VN_O_METHOD_PTYPE_STRING };
	VNOParam	value[3];
	char		buf[1300];
	size_t		len, to_go, chunk;

	for(to_go = len = strlen(text); to_go > 0; to_go -= chunk, text += chunk)	/* Split the text into chunks, if needed. Not very nice, but still. */
	{
		chunk = to_go > sizeof buf - 1 ? sizeof buf - 1 : to_go;
	
		memcpy(buf, text, chunk);
		buf[chunk] = '\0';

		value[0].vstring = (char *) channel;
		value[1].vstring = (char *) speaker;
		value[2].vstring = (char *) buf;

		if((pp = verse_method_call_pack(3, type, value)) != NULL)
			verse_send_o_method_call(nodedb_get_id(uc->node), uc->group_id, uc->method_id, ~0, pp);
	}
}

User * user_verse_new(const char *name, VNodeID node_id, uint16 group_id, uint16 method_id)
{
	UserClient	*uc;

	if((uc = malloc(sizeof *uc)) != NULL)
	{
		Node	*n;

		user_ctor(&uc->user, name);
		uc->group_id = group_id;
		uc->method_id = method_id;
		uc->user.hear = user_verse_hear;	/* Set superclass' hear() method. */
		if((n = nodedb_lookup(node_id)) != NULL)
		{
			if(nodedb_add_user_data(n, uc) == 0)
				fprintf(stderr, "**Error: user-verse got non-zero user data ID when attaching to node %u\n", node_id);
		}
		uc->node = n;
	}
	return (User *) uc;
}

User * user_verse_from_node_id(VNodeID sender)
{
	Node	*n;

	if((n = nodedb_lookup(sender)) != NULL)
	{
		UserClient	*uc;

		if((uc = nodedb_get_user_data(n, 0)) != NULL)
			return (User *) uc;
	}
	return NULL;
}
