#include <unistd.h>
#include <errno.h>
#include <string.h>

size_t confstr(int name, char *buf, size_t len)
{
  if (name == _CS_PATH)
    return strlcpy(buf, "/bin:/usr/bin", len);
  errno = EINVAL;
  return 0;
}
