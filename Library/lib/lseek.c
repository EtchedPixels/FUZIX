#include <stdlib.h>

off_t lseek(int fd, off_t pos, int whence)
{
  int e;
  off_t t = pos;
  e = _lseek(fd, &t, whence);
  if (e == -1)
    return (off_t)e;
  return t;
}