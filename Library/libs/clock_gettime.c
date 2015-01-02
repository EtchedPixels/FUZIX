#include <time.h>
#include <unistd.h>
#include <errno.h>

int clock_gettime(clockid_t clk_id, struct timespec *res)
{
  res->tv_nsec = 0;
  switch(clk_id) {
  case CLOCK_REALTIME:
    _time(&res->tv_sec, 0);
    return 0;
  case CLOCK_MONOTONIC:
    _time(&res->tv_sec, 1);
    return 0;
  default:
    errno = EINVAL;
  }
  return -1;
}