#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Divide *tp by 10 and also return the remainder */
/* On processors with real idiv we should probably just use that */
unsigned long div10quickm(unsigned long *tp)
{
	unsigned long q, r, t;
	t = *tp;
	q = (t >> 1) + (t >> 2);
	q = q + (q >> 4);
	q = q + (q >> 8);
	q = q + (q >> 16);
	q >>= 3;
	r = t - (((q << 2) + q) << 1);
	t = q + (r >> 9);
	*tp = t;
	t <<= 1;
	t = (t << 2) + t;
	return *tp - t;
}

int clock_gettime(clockid_t clk_id, struct timespec *res)
{
  int r;
  unsigned long d;
  res->tv_nsec = 0;
  switch(clk_id) {
  case CLOCK_REALTIME:
    _time(&res->tv_sec, 0);
    return 0;
  case CLOCK_MONOTONIC:
    _time(&res->tv_sec, 1);
    d = res->tv_sec;
    /* We know that this wraps at 2^32 ticks which also means we know
       it'll fit 32bits */
    r = div10quickm(&d);
    res->tv_nsec = 100000UL * r;
    res->tv_sec = d;
    return 0;
  default:
    errno = EINVAL;
  }
  return -1;
}