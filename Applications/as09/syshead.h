
#ifndef POSIX_HEADERS_MISSING
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifndef STDC_HEADERS_MISSING
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#endif

#ifdef MSDOS
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#undef min
#undef POSIX_HEADERS_MISSING
#define VERSION "MSDOS Compile"
#endif

#if __STDC__ && !defined(__minix)
#define P(x) x
#else
#define P(x) ()
#endif

#ifdef STDC_HEADERS_MISSING
char *strcpy P((char *s1, const char *s2));
char *strrchr P((const char *s, int c));
int memcmp P((const void *s1, const void *s2, unsigned n));
int strcmp P((const char *s1, const char *s2));
int strncmp P((const char *s1, const char *s2, unsigned n));
unsigned strlen P((const char *s));
void *malloc P((unsigned size));
void *memset P((void *s, int c, unsigned n));
void exit P((int status));
#endif

#ifdef POSIX_HEADERS_MISSING
int close P((int fd));
int creat P((const char *path, int mode));
int open P((const char *path, int oflag, ...));
int read P((int fd, void *buf, unsigned nbytes));
int write P((int fd, const void *buf, unsigned nbytes));
typedef long off_t;
off_t lseek P((int fd, off_t offset, int whence));
#define BIGBUFFER	0	/* Can't use a big buffer ... sorry */
#endif

#ifndef O_RDONLY
#define O_RDONLY	0
#endif
#ifndef O_WRONLY
#define O_WRONLY	1
#endif
#ifndef O_RDWR
#define O_RDWR		2
#endif
