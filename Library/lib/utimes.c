/*
 *	Fake utimes with utime. Our fs resolution is 1 second so its fine
 */

#include <sys/time.h>
#include <utime.h>

int utimes(const char *filename, const struct timeval times[2])
{
  struct utimbuf u;
  if (times == NULL)  
    return utime(filename, NULL);
  u.actime = times[0].tv_sec;
  u.modtime = times[0].tv_usec;
  return utime(filename, &u);
}