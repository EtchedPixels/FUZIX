#include <termios.h>
#include <unistd.h>

int tcdrain(int fd)
{
  struct termios t;
  if (ioctl(fd, TCGETS, &t))
    return -1;
  return ioctl(fd, TCSETSW, &t);
}
