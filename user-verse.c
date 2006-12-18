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

	printf("calling hear() in %u.%u.%u\n", uc->node_id, uc->group_id, uc->method_id);
	for(to_go = len = strlen(text); to_go > 0; to_go -= chunk, text += chunk)	/* Split the text into chunks, if needed. Not very nice, but still. */
	{
		chunk = to_go > sizeof buf - 1 ? sizeof buf - 1 : to_go;
	
		memcpy(buf, text, chunk);
		buf[chunk] = '\0';

		value[0].vstring = (char *) channel;
		value[1].vstring = (char *) speaker;
		value[2].vstring = (char *) buf;

		if((pp = verse_method_call_pack(3, type, value)) != NULL)
			verse_send_o_method_call(uc->node_id, uc->group_id, uc->method_id, ~0, pp);
	}
}

User * user_verse_new(const char *name, VNodeID node_id, uint16 group_id, uint16 method_id)
{
	UserClient	*uc;

	if((uc = malloc(sizeof *uc)) != NULL)
	{
		user_ctor(&uc->user, name);
		uc->node_id = node_id;
		uc->group_id = group_id;
		uc->method_id = method_id;
		uc->user.hear = user_verse_hear;
	}
	return (User *) uc;
}

/* This is O(n). Sad. */
User * user_verse_lookup(VNodeID node_id)
{
	unsigned int	i;
	User		*u;

	for(i = 0; (u = user_index(i)) != NULL; i++)
	{
		if(((UserClient *) u)->node_id == node_id)
			return u;
	}
	return NULL;
}
