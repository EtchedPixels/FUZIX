#include <time.h>
#include <unistd.h>
#include <errno.h>

int clock_getres(clockid_t clk_id, struct timespec *res)
{
  switch(clk_id) {
  case CLOCK_REALTIME:
    res->tv_nsec = 0;
    res->tv_sec = 1;
    return 0;
  case CLOCK_MONOTONIC:
    res->tv_nsec = 100000000UL;
    res->tv_sec = 0;
    return 0;
  default:
    errno = EINVAL;
  }
  return -1;
}