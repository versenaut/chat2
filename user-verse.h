/*
 * 
*/

#include "verse.h"

#include "user.h"

typedef struct {
	User	user;
	VNodeID	node_id;
	uint16	group_id;
	uint16	method_id;
} UserClient;

extern User *	user_verse_new(const char *name, VNodeID node_id, uint16 group_id, uint16 method_id);
