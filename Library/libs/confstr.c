#include <unistd.h>
#include <errno.h>

size_t confstr(int name, char *buf, size_t len)
{
  if (name == _CS_PATH)
    return strlcpy(buf, len, "/bin:/usr/bin");
  errno = EINVAL;
  return 0;
}
