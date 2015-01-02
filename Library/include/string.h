#ifndef __STRING_H
#define __STRING_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stddef.h>

/* Basic string functions */
extern size_t strlen __P ((const char* __str));

extern char * strcat __P ((char*, const char*));
extern char * strcpy __P ((char*, const char*));
extern int strcmp __P ((const char*, const char*));

extern char * strncat __P ((const char*, const char*, size_t));
extern char * strncpy __P ((const char*, const char*, size_t));
extern int strncmp __P((const char*, const char*, size_t));

extern int stricmp __P((const char*, const char*));
extern strnicmp __P((const char*, const char*, size_t));

extern int strcasecmp __P((const char*, const char*));
extern strncasecmp __P((const char*, const char*, size_t));

extern char * strchr __P ((const char*, int));
extern char * strrchr __P ((const char*, int));
extern char * strdup __P ((const char*));

/* Basic mem functions */
extern void * memcpy __P ((void*, const void*, size_t));
extern void * memccpy __P ((void*, const void*, int, size_t));
extern void * memchr __P ((const void*, int, size_t));
extern void * memset __P ((void*, int, size_t));
extern int memcmp __P ((const void*, const void*, size_t));

extern void * memmove __P ((void*, const void*, size_t));

/* BSDisms */
extern char *index __P ((const char *, int));
extern char *rindex __P ((const char *, int));
extern void bcopy __P ((const void*, void*, size_t));
extern void bzero __P ((void*, int));

/* Othe common BSD functions */
extern char *strpbrk __P ((const char *, const char *));
extern char *strsep __P ((char **, const char *));
extern char *strstr __P ((const char *, const char *));
extern char *strtok __P ((char *, const char *));
extern size_t strcspn __P ((const char *, const char *));
extern size_t strspn __P ((const char *, const char *));

extern size_t strlcpy __P((char *, const char *, size_t));
extern size_t strlcat __P((char *, const char *, size_t));

extern char *strcasestr __P((const char *, const char *));

#ifdef	z80
#pagma inline(memcpy)
#pagma inline(memset)
#pagma inline(strcpy)
#pagma inline(strlen)
#pagma inline(strcmp)
#endif

#endif		/* END OF DEFINITION	STRING.H */
