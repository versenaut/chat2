/*
 * 
*/

#if !defined NODEDB_H
#define	NODEDB_H

#include "verse.h"

typedef struct Node	Node;

extern void		nodedb_init(void);

extern Node *		nodedb_new(VNodeID node_id);
extern VNodeID		nodedb_get_id(const Node *node);
extern void		nodedb_set_name(Node *node, const char *name);
extern const char *	nodedb_get_name(const Node *node);
extern int		nodedb_add_user_data(Node *node, const void *data);
extern void *		nodedb_get_user_data(Node *node, int index);
extern Node *		nodedb_lookup(VNodeID node_id);

#endif		/* NODEDB_H */
