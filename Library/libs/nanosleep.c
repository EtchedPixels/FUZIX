#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Ok so this doesn't score too highly on accuracy but it'll wrap code using
   the modern API's for long sleeps just fine */

int clock_nanosleep(clockid_t clock, int flags, const struct timespec *req, struct timespec *rem)
{
  time_t tbase;
  time_t tend;
  uint16_t t = req->tv_sec * 10;
  long nsec = req->tv_nsec;

  if (clock != CLOCK_REALTIME && clock != CLOCK_MONOTONIC) {
   errno = EINVAL;
   return -1;
  }

  /* Will be 0-9 range so avoid costly divides */
  while(nsec > 50000000UL) {
    nsec -= 50000000UL;
    t++;
  }
 
  if (_time(&tbase, clock) == -1)
    return -1;

  if (flags & TIMER_ABSTIME) {
   if (tbase < req->tv_sec)
    return 0;
   t += (tbase - req->tv_sec) * 10;
  }
  if (_pause(t) == 0)
    return 0;
  _time(&tend, clock);
  if (rem) {
    rem->tv_sec = tbase + t/10 - tend;
    rem->tv_nsec = 0;
  }
  return -1;
}


int nanosleep(const struct timespec *req, struct timespec *rem)
{
 return clock_nanosleep(CLOCK_REALTIME, 0, req, rem);
}

