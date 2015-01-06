#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Ok so this doesn't score too highly on accuracy but it'll wrap code using
   the modern API's for long sleeps just fine */

int clock_nanosleep(clockid_t clock, int flags, const struct timespec *reqp, struct timespec *rem)
{
  struct timespec tbase, tend, req;
  uint16_t t;

  if (clock_gettime(clock, &tbase))
   return -1;
 
  req.tv_sec = reqp->tv_sec;
  req.tv_nsec = reqp->tv_nsec;

  /* Absolute time means we are handed a time relative to system 0 not
     a time relative to 'now' */
  if (flags & TIMER_ABSTIME) {
    req.tv_sec -= tbase.tv_sec;
    req.tv_nsec -= tbase.tv_nsec;
    if (req.tv_nsec < 0) {
     req.tv_nsec += 1000000L;
     --req.tv_sec;
    }
    if (req.tv_sec < 0)
     return 0;
    /* req is now the relative time for all cases */
  }
  /* Convert duration into 16bit 10ths of a second */
  t = ((uint16_t)req.tv_sec) * 10;
  while(req.tv_nsec > 100000000UL) {
    req.tv_nsec -= 100000000UL;
    t++;
  }
  if (t == 0 || _pause(t) == 0)
    return 0;
  /* When did we finish */
  clock_gettime(clock, &tend);

  /* Work out the time left avoiding divisions */
  if (!(flags  & TIMER_ABSTIME)) {
    tend.tv_sec -= tbase.tv_sec;
    tend.tv_nsec -= tbase.tv_nsec;
    if (tend.tv_nsec < 0) {
     tend.tv_sec--;
     tend.tv_nsec += 1000000000UL;
   }
   if (tend.tv_sec < 0)
      return 0;
   if (rem) {
     rem->tv_sec = tend.tv_sec;
     rem->tv_nsec = tend.tv_nsec;
   }
  } else {
    /* In absolute mode rem is req, easy as that */
    if (rem) {
      rem->tv_sec = reqp->tv_sec;
      rem->tv_nsec = reqp->tv_nsec;
    }
  }
  return -1;
}


int nanosleep(const struct timespec *req, struct timespec *rem)
{
 return clock_nanosleep(CLOCK_REALTIME, 0, req, rem);
}

