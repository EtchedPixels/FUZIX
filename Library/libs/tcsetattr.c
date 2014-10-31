#include <termios.h>
#include <unistd.h>


int tcsetattr(int fd, int oa, const struct termios *termios_p)
{
  int op = TCSETS;
  if (oa == TCSADRAIN)
    op = TCSETSW;
  if (oa == TCSAFLUSH)
    op = TCSETSF;
  return ioctl(fd, op, termios_p);
}

