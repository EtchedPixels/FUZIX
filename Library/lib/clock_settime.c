#include <time.h>
#include <unistd.h>
#include <errno.h>

int clock_settime(clockid_t clk_id, const struct timespec *tp)
{
  switch(clk_id) {
  case CLOCK_REALTIME:
    _stime(&tp->tv_sec, 0);
    return 0;
  case CLOCK_MONOTONIC:
    return -EPERM;
  default:
    errno = EINVAL;
  }
  return -1;
}