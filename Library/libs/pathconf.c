#include <unistd.h>
#include <errno.h>

long _pathconf(int name)
{
  struct _uzisysinfoblk info;
  
  _uname(&info, sizeof(info));

  switch(name) {
    case _PC_LINK_MAX:
      return 65535;
    case _PC_MAX_CANON:
    case _PC_MAX_INPUT:
      return 132;		/* In theory must be 256+ .. */
    case _PC_NAME_MAX:
      return 30;
    case _PC_PATH_MAX:
      return 512;
    case _PC_PIPE_BUF:
      return 4096;		/* FIXME: wrong but need to sort socket
                                   buffers out to know the right answers */
    case _PC_CHOWN_RESTRICTED:
      return 1;
    case _PC_NO_TRUNC:
      return 0;
    case _PC_VDISABLE:
      return 1;
  }
  return -1;
}

long pathconf(char *p, int name)
{
  p;
  return _pathconf(name);
}

long fpathconf(int fd, int name)
{
  fd;
  return _pathconf(name);
}
