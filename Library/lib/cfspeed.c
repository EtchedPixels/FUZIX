#include <termios.h>
#include <unistd.h>

speed_t cfgetispeed(const struct termios *p)
{
  return p->c_cflag&CBAUD;
}

speed_t cfgetospeed(const struct termios *p)
{
  return p->c_cflag&CBAUD;
}

int cfsetspeed(struct termios *p, speed_t s)
{
  if (s & ~CBAUD)	/* Invalid bits ? */
    return -1;
  p->c_cflag &= ~CBAUD;
  p->c_cflag |= s;
  return 0;
}

int cfsetispeed(struct termios *p, speed_t s)
{
  return cfsetspeed(p,s);
}

int cfsetospeed(struct termios *p, speed_t s)
{
  return cfsetspeed(p,s);
}
