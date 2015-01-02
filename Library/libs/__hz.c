#include <unistd.h>
#include <time.h>

static int hz;

int __hz(void)
{
  if (!hz) {
    struct _uzisysinfoblk info;
    _uname(&info, sizeof(info));
    hz = info.ticks;
  }
  return hz;
}

  
