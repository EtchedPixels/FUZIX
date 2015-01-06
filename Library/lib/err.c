#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

extern char **__argv;

static void _vdo(int err, int flags, const char *fmt,  va_list args)
{
  fprintf(stderr, "%s", __argv[0]);
  if (fmt) {
    vfprintf(stderr, fmt, args);
    if (flags & 1)
      fprintf(stderr, ": %s", strerror(err));
    fputc('\n', stderr);
  }
}

void verr(int eval, const char *fmt, va_list args)
{
  _vdo(errno, 1, fmt, args);
  exit(eval);
}

void verrc(int err, int eval, const char *fmt, va_list args)
{
  _vdo(err, 1, fmt, args);
  exit(eval);
}

void verrx(int eval, const char *fmt, va_list args)
{
  _vdo(errno, 0, fmt, args);
  exit(eval);
}

void vwarn(const char *fmt, va_list args)
{
  _vdo(errno, 1, fmt, args);
}

void vwarnc(int err, const char *fmt, va_list args)
{
  _vdo(err, 1, fmt, args);
}

void vwarnx(const char *fmt, va_list args)
{
  _vdo(errno, 0, fmt, args);
}

void err(int eval, const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(errno, 1, fmt, ptr);
  exit(eval);
}

void errc(int eval, int err, const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(err, 1, fmt, ptr);
  exit(eval);
}

void errx(int eval, const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(errno, 0, fmt, ptr);
  exit(eval);
}

void warn(const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(errno, 1, fmt, ptr);
}

void warnc(int err, const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(err, 1, fmt, ptr);
}

void warnx(const char *fmt, ...)
{
  va_list ptr;
  va_start(ptr, fmt);
  _vdo(errno, 0, fmt, ptr);
}

