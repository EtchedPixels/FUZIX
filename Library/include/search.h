#ifndef __SEARCH_H
#define __SEARCH_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stddef.h>

#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (*__compar_fn_t) __P((__ptr_t, __ptr_t));
#endif

/* for use with hsearch(3) */

<<<<<<< HEAD
typedef struct entry {
    char *key;
    char *data;
} ENTRY;
=======
typedef struct entry { char *key; char *data; } ENTRY;
>>>>>>> e059da05c2a2aa98ddc3528dd44c116c02b12a91
typedef enum { FIND, ENTER } ACTION;

extern ENTRY * hsearch __P((ENTRY __item, ACTION __action));
extern int     hcreate __P((unsigned __nel));
extern void    hdestroy __P((void));

/* The tsearch routines are very interesting. They make many
 * assumptions about the compiler. It assumpts that the first field
 * in node must be the "key" field, which points to the datum.
 * Everything depends on that. It is a very tricky stuff. H.J.
 */
/* For tsearch */
typedef enum { preorder, postorder, endorder, leaf } VISIT;

extern void *tsearch __P((void *__key, void **__rootp, __compar_fn_t compar));

extern void *tfind __P((void *__key, void ** __rootp, __compar_fn_t compar));

extern void *tdelete __P((void * __key, void ** __rootp, __compar_fn_t compar));

#ifndef __ACTION_FN_T
#define __ACTION_FN_T
/* FYI, the first argument should be a pointer to "key".
 * Please read the man page for details.
 */
typedef void (*__action_fn_t) __P((void *__nodep, VISIT __value, int __level));
#endif

extern void twalk __P((void * __root, __action_fn_t action));


extern void * lfind __P((void * __key, void * __base,
<<<<<<< HEAD
                         size_t * __nmemb, size_t __size,
                         __compar_fn_t __compar));

extern void * lsearch __P((void * __key, void * __base,
                           size_t * __nmemb, size_t __size,
                           __compar_fn_t __compar));
=======
			 size_t * __nmemb, size_t __size,
			 __compar_fn_t __compar));

extern void * lsearch __P((void * __key, void * __base,
			 size_t * __nmemb, size_t __size,
			 __compar_fn_t __compar));
>>>>>>> e059da05c2a2aa98ddc3528dd44c116c02b12a91

#endif /* search.h */
