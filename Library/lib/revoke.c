#include <unistd.h>
#include <termios.h>

int frevoke(int fd)
{
  return ioctl(fd, TIOCHANGUP, 0);
}
