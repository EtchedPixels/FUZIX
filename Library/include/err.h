#ifndef _ERR_H
#define _ERR_H

#include <stdarg.h>
/* BSD err functionality */

extern void err(int eval, const char *fmt, ...);
extern void errc(int eval, int code, const char *fmt, ...);
extern void errx(int eval, const char *fmt, ...);
extern void warn(const char *fmt, ...);
extern void warnc(int code, const char *fmt, ...);
extern void warnx(const char *fmt, ...);

extern void verr(int eval, const char *fmt, va_list args);
extern void verrc(int eval, int code, const char *fmt, va_list args);
extern void verrx(int eval, const char *fmt, va_list args);
extern void vwarn(const char *fmt, va_list args);
extern void vwarnc(int code, const char *fmt, va_list args);
extern void vwarnx(const char *fmt, va_list args);

#endif
