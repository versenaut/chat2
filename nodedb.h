/*
 * 
*/

#include "verse.h"

typedef struct Node	Node;

extern void		nodedb_init(void);

extern Node *		nodedb_new(VNodeID node_id);
extern void		nodedb_set_name(Node *node, const char *name);
extern const char *	nodedb_get_name(const Node *node);
extern Node *		nodedb_lookup(VNodeID node_id);
