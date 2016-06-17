#ifndef _ERR_H
#define _ERR_H

#include <stdarg.h>
/* BSD err functionality */

extern void err(int __eval, const char *__fmt, ...);
extern void errc(int __eval, int __code, const char *__fmt, ...);
extern void errx(int __eval, const char *__fmt, ...);
extern void warn(const char *__fmt, ...);
extern void warnc(int __code, const char *__fmt, ...);
extern void warnx(const char *__fmt, ...);

extern void verr(int __eval, const char *__fmt, va_list __args);
extern void verrc(int __eval, int __code, const char *__fmt, va_list __args);
extern void verrx(int __eval, const char *__fmt, va_list __args);
extern void vwarn(const char *__fmt, va_list __args);
extern void vwarnc(int __code, const char *__fmt, va_list __args);
extern void vwarnx(const char *__fmt, va_list __args);

#endif
