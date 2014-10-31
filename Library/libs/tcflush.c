#include <termios.h>
#include <unistd.h>

int tcflush(int fd, int q)
{
  return ioctl(fd, TIOCFLUSH, q);
}
