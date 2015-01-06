#define _BSD_SOURCE

#include <sys/time.h>
#include <unistd.h>

int settimeofday(struct timeval *tv, const struct timezone *tz)
{
  int ret = 0;
  if (tv) {
    ret = _stime(&tv->tv_sec, 0);
  }
  return ret;
}
