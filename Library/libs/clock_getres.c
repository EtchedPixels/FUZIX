#include <time.h>
#include <unistd.h>
#include <errno.h>

int clock_getres(clockid_t clk_id, struct timespec *res)
{
  res->tv_nsec = 0;
  res->tv_sec = 1;
  switch(clk_id) {
  case CLOCK_REALTIME:
  case CLOCK_MONOTONIC:
    return 0;
  default:
    errno = EINVAL;
  }
  return -1;
}