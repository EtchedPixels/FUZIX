#ifndef __MALLOC_H
#define __MALLOC_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern void free __P((void *));
extern void *malloc __P((size_t));
extern void *realloc __P((void *, size_t));
extern void *calloc __P((size_t, size_t));
extern void *alloca __P((size_t));

#endif
