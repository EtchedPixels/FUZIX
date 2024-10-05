#include <stdlib.h>

int ftruncate(int fd, off_t pos)
{
  return _ftruncate(fd, &pos);
}