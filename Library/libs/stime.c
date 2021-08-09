#include <stdio.h>
#include <unistd.h>

/*
 *	Wrap the kernel time call so that it also
 *	returns a time_t (longlong). The kernel ABI
 *	doesn't deal in 64bit return values.
 */
int stime(const time_t *t)
{
  __ktime_t tmp;
  tmp.low = *t;
#ifndef NO_64BIT
  tmp.high = *t >> 32;
#endif
  return _stime(&tmp, 0);
}
