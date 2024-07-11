#ifndef __STRING_H
#define __STRING_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stddef.h>

/* Basic string functions */
extern size_t strlen(const char *__s);

extern char *strcat(char *__dest, const char *__src);
extern char *strcpy(char *__dest, const char *__src);
extern int strcmp(const char *__s1, const char *__s2);

extern char *strncat(char *__dest, const char *__src, size_t __n);
extern char *strncpy(char *__dest, const char *__src, size_t __n);
extern int strncmp(const char *__s1, const char *__s2, size_t __n);

extern int stricmp(const char *__s1, const char *__s2);
extern int strnicmp(const char *__s1, const char *__s2, size_t __n);

extern int strcasecmp(const char *__s1, const char *__s2);
extern int strncasecmp(const char *__s1, const char *__s2, size_t __n);

extern char *strchr(const char *__s, int __c);
extern char *strrchr(const char *__s , int __c);
extern char *strdup(const char *__s);
/* FIXME: missing but in POSIX */
extern char *strndup(const char *__s, int __n);

/* Basic mem functions */
extern void *memcpy(void *__dest, const void *__src, size_t __n);
extern void *memccpy(void *__dest, const void *__src, int __c, size_t __n);
extern void *memchr(const void *__src, int __c, size_t __n);
extern void *memset(void *__s, int __c, size_t __n);
extern int memcmp(const void *__s1, const void *__s2, size_t __n);

extern void *memmove(void *__dest, const void *__src, size_t __n);

/* BSDisms */
extern char *index(const char *__s, int __c);
extern char *rindex(const char *__s, int __c);
extern void bcopy(const void *__src, void *__dst, size_t __n);
extern void bzero(void *__dst, size_t __n);
extern int bcmp(const void *__s1, const void *__s2, size_t __n);

/* Other common string functions */
extern char *strpbrk(const char *__s, const char *__accept);
extern char *strsep(char **__stringp, const char *__delim);
extern char *strstr(const char *__haystack, const char *__needle);
extern char *strtok(char *__str, const char *__delim);
extern char *strtok_r(char *__str, const char *__delim, char **__olds);
extern size_t strcspn(const char *__s, const char *__reject);
extern size_t strspn(const char *__s, const char *__accept);

extern size_t strlcpy(char *__dest, const char *__src, size_t __maxlen);
extern size_t strlcat(char *__dest, const char *__src, size_t __maxlen);

/* FIXME: GNUism */
extern char *strcasestr(const char *__needle, const char *__haystack);

/* Later ISOisms */
extern size_t strnlen(const char *__s, size_t __maxlen);
extern size_t strxfrm(char *__dest, const char *__src, size_t __n);
extern int strcoll(const char *__s1, const char *__s2);

extern const char *strsignal(int __sig);
extern char *strerror(int __errno);

#endif		/* END OF DEFINITION	STRING.H */
