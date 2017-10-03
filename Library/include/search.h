#ifndef __SEARCH_H
#define __SEARCH_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stddef.h>

#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (*__compar_fn_t)(__ptr_t, __ptr_t);
#endif

/* for use with hsearch(3) */

typedef struct entry { char *key; char *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

/* Some of our compilers can't do struct arguments at all */
extern ENTRY * __hsearch(ENTRY *__item, ACTION __action);
#define hsearch(a,b,c)	__hsearch(&(a),(b),(c))
extern int     hcreate(unsigned __nel);
extern void    hdestroy(void);

/* The tsearch routines are very interesting. They make many
 * assumptions about the compiler. It assumpts that the first field
 * in node must be the "key" field, which points to the datum.
 * Everything depends on that. It is a very tricky stuff. H.J.
 */
/* For tsearch */

typedef enum { preorder, postorder, endorder, leaf } VISIT;

extern void *tsearch(void *__key, void **__rootp, __compar_fn_t compar);
extern void *tfind(void *__key, void ** __rootp, __compar_fn_t compar);
extern void *tdelete(void * __key, void ** __rootp, __compar_fn_t compar);

#ifndef __ACTION_FN_T
#define __ACTION_FN_T
/* FYI, the first argument should be a pointer to "key".
 * Please read the man page for details.
 */
typedef void (*__action_fn_t)(void *__nodep, VISIT __value, int __level);
#endif

extern void twalk(void * __root, __action_fn_t action);


extern void * lfind(void * __key, void * __base,
			 size_t * __nmemb, size_t __size,
			 __compar_fn_t __compar);

extern void * lsearch(void * __key, void * __base,
			 size_t * __nmemb, size_t __size,
			 __compar_fn_t __compar);

#endif /* search.h */
