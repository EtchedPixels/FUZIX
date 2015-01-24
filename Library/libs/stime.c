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
  tmp.time = *t;
#if defined(NO_64BIT)
  tmp.pad = 0;
#endif
  return _stime(&tmp, 0);
}
