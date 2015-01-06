#define _BSD_SOURCE

#include <sys/time.h>
#include <unistd.h>

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  if (tv) {
    _time(&tv->tv_sec, 0);
    tv->tv_usec = 0;
  }
  if (tz) {
    /* FIXME: although this is obsolete */
    tz->tz_minuteswest = 0;
    /* and this is not used */
    tz->tz_dsttime = 0;
  }
  return 0;
}

  