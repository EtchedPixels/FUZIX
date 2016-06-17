#ifndef __UTIME_H
#define __UTIME_H
#ifndef __TYPES_H
#include <types.h>
#endif

struct utimbuf {
    time_t actime;  /* access time */
    time_t modtime; /* modification time */
};

extern int utime(const char *__filename, const struct utimbuf *__utimebuf);

#endif
