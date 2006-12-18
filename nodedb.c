/*
 * 
*/

#include <stdio.h>
#include <string.h>

#include "verse.h"

#include "qsarr.h"

#include "nodedb.h"

#if defined _WIN32
#define	snprintf	_snprintf
#endif

struct Node {
	VNodeID	node_id;
	char	name[32];
};

static struct {
	QSArr	*nodes;
} NodeInfo;

/*------------------------------------------------------------------------------------------------ */

static int cmp_node_sort(const void **e1, const void **e2)
{
	const Node	*na = *(Node **) e1, *nb = *(Node **) e2;

	return na->node_id < nb->node_id ? -1 : na->node_id > nb->node_id;	/* As close to <=> as we get, in C. :) */
}

static int cmp_node_key(const void *n, const void *key)
{
	const Node	*node = n;

	return node->node_id < (VNodeID) key ? -1 : node->node_id > (VNodeID) key;
}

void nodedb_init(void)
{
	NodeInfo.nodes = qsarr_new(cmp_node_sort, cmp_node_key);
}

Node * nodedb_new(VNodeID node_id)
{
	Node	*n;

	if((n = malloc(sizeof *n)) != NULL)
	{
		n->node_id = node_id;
		snprintf(n->name, sizeof n->name, "node-%u", node_id);
		qsarr_insert(NodeInfo.nodes, n);
	}
	return n;
}

/* Like the server itself, this doesn't actually check against name collisions. */
void nodedb_set_name(Node *node, const char *name)
{
	if(node == NULL || name == NULL)
		return;
	snprintf(node->name, sizeof node->name, "%s", name);
}

const char * nodedb_get_name(const Node *node)
{
	if(node == NULL)
		return NULL;
	return node->name;
}

Node * nodedb_lookup(VNodeID node_id)
{
	return qsarr_lookup(NodeInfo.nodes, (void *) node_id);
}
