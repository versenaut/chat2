/*
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qsarr.h"

/*------------------------------------------------------------------------------------------------ */

struct QSArr {
	void	**data;
	size_t	size, alloc;

	int	(*cmp_sort)(const void **e1, const void **e2);
	int	(*cmp_key)(const void *e, const void *key);
};

/*------------------------------------------------------------------------------------------------ */

QSArr * qsarr_new(int (*cmp_sort)(const void **e1, const void **e2),
		  int (*cmp_key)(const void *e, const void *key))
{
	QSArr	*qsa;

	if((qsa = malloc(sizeof *qsa)) != NULL)
	{
		qsa->data = NULL;
		qsa->size = 0;
		qsa->alloc = 0;
		qsa->cmp_sort = cmp_sort;
		qsa->cmp_key = cmp_key;
	}
	return qsa;
}

static int grow(QSArr *qsa)
{
	size_t	ns;
	void	**nd;

	ns = 2 * qsa->alloc;
	if(ns == 0)
		ns = 8;
	if((nd = realloc(qsa->data, ns * sizeof *qsa->data)) != NULL)
	{
		qsa->data = nd;
		qsa->alloc = ns;
		return 1;
	}
	return 0;
}

static void resort(QSArr *qsa)
{
	qsort(qsa->data, qsa->size, sizeof *qsa->data, (int (*)(const void *, const void *)) qsa->cmp_sort);
}

void qsarr_insert(QSArr *qsa, void *element)
{
	if(qsa->size >= qsa->alloc)
		if(!grow(qsa))
			return;
	qsa->data[qsa->size++] = element;
	resort(qsa);
	printf("qsa now has %u elements\n", qsa->size);
}

static int find(const QSArr *qsa, const void *key)
{
	size_t	mn, mx, md;
	int	c;

	if(qsa == NULL || key == NULL)
		return -1;

	for(mn = 0, mx = qsa->size; mn <= mx;)
	{
		md = (mn + mx) / 2;
		c = qsa->cmp_key(qsa->data[md], key);
		if(c == 0)
			return (int) md;
		if(c > 0)
			mx = md - 1;
		else
			mn = md + 1;
	}
	return -1;
}

void * qsarr_lookup(const QSArr *qsa, const void *key)
{
	int	index = find(qsa, key);

	if(index >= 0)
		return qsa->data[index];
	return NULL;
}

void qsarr_remove(QSArr *qsa, const void *key)
{
	int	index = find(qsa, key);

	if(index >= 0)
	{
		if(qsa->size > 1)
			qsa->data[index] = qsa->data[qsa->size - 1];
		qsa->size--;
		if(qsa->size > 1)
			resort(qsa);
	}
}

void * qsarr_index(const QSArr *qsa, unsigned int index)
{
	if(qsa == NULL || index >= qsa->size)
		return NULL;
	return qsa->data[index];
}

size_t qsarr_size(const QSArr *qsa)
{
	return qsa != NULL ? qsa->size : 0;
}
