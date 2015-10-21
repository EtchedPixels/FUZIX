#include <stdio.h>
#include <paths.h>
#include <fcntl.h>
#include <unistd.h>

/*
 *	Emulate Berkley hostname calls
 */

static int openhostname(int flags, mode_t mode)
{
  return open(_PATH_HOSTNAME, flags, mode);
}
  
int gethostname(char *name, size_t len)
{
  int got;
  int fd = openhostname(O_RDONLY, 0);
  if (fd == -1) {
    *name = 0;
    return 0;
  }
  got = read(fd, name, len);
  if (got >= 0)
    name[got] = 0;
  else
    *name = 0;
  close(fd);
  return 0;
}

int sethostname(const char *name, size_t len)
{
  int fd = openhostname(O_TRUNC|O_CREAT, 0644);
  if (fd == -1)
    return -1;
  /* Not clear how to handle this */
  if (write(fd, name, len) != len)
    return -1;
  fchmod(fd, 0644);
  close(fd);
  return 0;
}
