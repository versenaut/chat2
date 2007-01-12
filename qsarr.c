/*
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qsarr.h"

/*------------------------------------------------------------------------------------------------ */

struct QSArr {
	void		**data;
	int		size, alloc;
	size_t		offset;		/* Offset to string embedded in user element, for string mode. */
	int		frozen;
	unsigned int	dirty : 1;	/* Array was touched when frozen; resort needed on thaw. */

	int		(*cmp_sort)(const void **e1, const void **e2);
	int		(*cmp_key)(const void *e, const void *key);
};

/*------------------------------------------------------------------------------------------------ */

#define	EL(i)		((char *) base + i * width)
#define	EL_STACK	512

/* A qsort() implementation, that passes a 'context' void pointer to the comparison callback.
 * Adapted from the C code at <http://en.wikipedia.org/wiki/Quicksort>. Note that we need to
 * store the 'pivot' element during the loop, which is what the <pe> provides a buffer for.
*/
static void q_sort(void *base,
		   size_t left,
		   size_t right,
		   size_t width,
		   int (*compare)(void *context, const void *a, const void *b),
		   void *context,
		   void *pe)
{
	size_t	l_hold = left, r_hold = right, pivot = left;

	memcpy(pe, EL(pivot), width);
	while(left < right)
	{
		while(compare(context, EL(right), pe) >= 0 && left < right)
			right--;
		if(left != right)
		{
			memcpy(EL(left), EL(right), width);
			left++;
		}
		while(compare(context, EL(left), pe) <= 0 && left < right)
			left++;
		if(left != right)
		{
			memcpy(EL(right), EL(left), width);
			right--;
		}
	}
	memcpy(EL(left), pe, width);
	pivot = left;
	left = l_hold;
	right = r_hold;
	if(left < pivot)
		q_sort(base, left, pivot - 1,  width, compare, context, pe);
	if(right > pivot)
		q_sort(base, pivot + 1, right, width, compare, context, pe);
}

int qsort_s(void *base,
	     size_t num,
	     size_t width,
	     int (*compare)(void *context, const void *a, const void *b),	/* Should return a <=> b. */
	     void *context)
{
	if(base == NULL || num < 2 || width == 0 || compare == NULL)
		return 0;
	/* Use buffer space on-stack for small(ish) elements, malloc() it for larger. */
	if(width <= EL_STACK)
	{
		char	buffer[EL_STACK];

		q_sort(base, 0, num - 1, width, compare, context, buffer);
		return 1;
	}
	else
	{
		char	*buffer;

		if((buffer = malloc(width)) != NULL)
		{
			q_sort(base, 0, num - 1, width, compare, context, buffer);
			free(buffer);
		}
		return buffer != NULL;
	}
}

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
		qsa->frozen = 0;
		qsa->dirty = 0;
		qsa->cmp_sort = cmp_sort;
		qsa->cmp_key = cmp_key;
	}
	return qsa;
}

/*
static int cmp_string_offset(void *context, const void **e1, const void **e2)
{
	const char	*s1 = *(char **) ((char *) e1 + ((QSArr *) context)->offset),
			*s2 = *(char **) ((char *) e2 + ((QSArr *) context)->offset);

	return strcmp(s1, s2);		  
}
*/

void qsarr_destroy(QSArr *qsa)
{
	if(qsa == NULL)
		return;
	if(qsa->data != NULL)
		free(qsa->data);
	free(qsa);
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
	if(qsa->frozen == 0)
		resort(qsa);
	else
		qsa->dirty = 1;
}

void qsarr_freeze(QSArr *qsa)
{
	qsa->frozen++;
}

void qsarr_thaw(QSArr *qsa)
{
	if(--qsa->frozen == 0)
	{
		if(qsa->dirty)
			resort(qsa);
		qsa->dirty = 0;
	}
}

static int find(const QSArr *qsa, const void *key)
{
	int	mn, mx, md, c;

	if(qsa == NULL || qsa->size == 0)
		return -1;

	for(mn = 0, mx = qsa->size - 1; mn <= mx;)
	{
		md = (mn + mx) / 2;
		c = qsa->cmp_key(qsa->data[md], key);
/*		printf("mn=%u mx=%u -> md=%u, c=%d\n", mn, mx, md, c);*/
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
		{
			if(qsa->frozen == 0)
				resort(qsa);
			else
				qsa->dirty = 1;
		}
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
