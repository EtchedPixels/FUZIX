#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

extern char **__argv;

static void _vdo(int flags, const char *fmt,  va_list args)
{
  fprintf(stderr, "%s", __argv[0]);
  if (fmt) {
    vfprintf(stderr, fmt, args);
    if (flags & 1)
      fprintf(stderr, ": %s", strerror(errno));
    fputc('\n', stderr);
  }
}

void verr(int eval, const char *fmt, va_list args)
{
  _vdo(1, fmt, args);
  exit(eval);
}

void verrx(int eval, const char *fmt, va_list args)
{
  _vdo(0, fmt, args);
  exit(eval);
}

void vwarn(const char *fmt, va_list args)
{
  _vdo(1, fmt, args);
}

void vwarnx(const char *fmt, va_list args)
{
  _vdo(0, fmt, args);
}

void err(int eval, const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(1, fmt, ptr);
  exit(eval);
}

void errx(int eval, const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(0, fmt, ptr);
  exit(eval);
}

void warn(const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(1, fmt, ptr);
}

void warnx(const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(0, fmt, ptr);
}

