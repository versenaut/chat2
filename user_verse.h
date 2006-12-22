/*
 * 
*/

#include "verse.h"

#include "nodedb.h"
#include "user.h"

typedef struct {
	User	user;
	uint16	group_id;
	uint16	method_id;
	Node	*node;
} UserVerse;

extern User *	user_verse_new(const char *name, VNodeID node_id, uint16 group_id, uint16 method_id);

extern User *	user_verse_from_node_id(VNodeID sender);
