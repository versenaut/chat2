/*
 * A quick (?), sorted pointer array. Makes me sleep a bit better.
*/

typedef struct QSArr	QSArr;

extern QSArr *	qsarr_new(int (*cmp_sort)(const void **e1, const void **e2),
			  int (*cmp_key)(const void *e, const void *key));
extern QSArr *	qsarr_new_string(size_t string_offset);
extern void	qsarr_destroy(QSArr *qsa);
extern void	qsarr_set_comparison_function(QSArr *qsa, int (*cmp)(void *a, void *b));

extern void *	qsarr_lookup(const QSArr *qsa, const void *key);
extern void *	qsarr_index(const QSArr *qsa, unsigned int index);

extern void	qsarr_freeze(QSArr *qsa);
extern void	qsarr_thaw(QSArr *qsa);

extern size_t	qsarr_size(const QSArr *qsa);
