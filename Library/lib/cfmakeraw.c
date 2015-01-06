#include <termios.h>
#include <unistd.h>

void cfmakeraw(struct termios *p)
{
  p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXANY | IXOFF);
  p->c_oflag &= ~OPOST;
  p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  p->c_cflag &= ~(CSIZE | PARENB);
  p->c_cflag |= CS8;
}
