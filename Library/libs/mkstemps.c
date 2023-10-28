#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int mkstemps(char *s, int slen)
{
  __ktime_t t;
  char *p = s + strlen(s) - slen - 6;
  uint16_t value;
  const char *n;
  int fd;

  if (p < s)
    goto bad;
  if (memcmp(p, "XXXXXX", 6))
    goto bad;
  _time(&t, 0);
  value = (getuid() << 8) + getpid() + (uint16_t)t.low;
  do {
    value += 7919;	/* Any old prime ought to do */
    n = _itoa(value);
    memcpy(p, "000000", 6);
    memcpy(p + 6 - strlen(n), n, strlen(n));
    fd = open(s, O_CREAT|O_EXCL|O_RDWR, 0600);
  }
  while(fd == -1 && errno == EEXIST);
  return fd;
bad:
  errno = EINVAL;
  return -1;
}

int mkstemp(char *s)
{
  return mkstemps(s, 0);
}
