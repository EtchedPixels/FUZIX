#include <stdio.h>
#include <unistd.h>

/*
 *	Wrap the kernel time call so that it also
 *	returns a time_t (longlong). The kernel ABI
 *	doesn't deal in 64bit return values.
 */
time_t time(time_t *t)
{
  time_t tmp;
  if (t) {
    _time(t, 0);
    return *t;
  }
  _time(&tmp, 0);
  return tmp;
}
