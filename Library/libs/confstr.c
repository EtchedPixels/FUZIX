#include <unistd.h>
#include <errno.h>

size_t confstr(int name, char *buf, size_t len)
{
  errno = EINVAL;
  return 0;
}
