#ifndef __MALLOC_H
#define __MALLOC_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern void free(void *__ptr);
extern void *malloc(size_t __size);
extern void *realloc(void *__ptr, size_t __size);
extern void *calloc(size_t __nmemb, size_t __size);

#endif
