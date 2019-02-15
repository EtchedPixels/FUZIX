#include <unistd.h>
#include <errno.h>

long sysconf(int name)
{
  struct _uzisysinfoblk info;
  unsigned long psize;
  int pscale;
  
  _uname(&info, sizeof(info));
  psize = 65536/info.banks;
  pscale = psize/1024;
  
  switch(name) {
    case _SC_ARG_MAX:
      return 512;
    case _SC_CHILD_MAX:
      return info.nproc - 1;
    case _SC_HOST_NAME_MAX:
      /* we will implement get/sethostname and domain name in userspace */
      return 256;
    case _SC_LOGIN_NAME_MAX:
      return 32;
    case _SC_CLK_TCK:
      /* query via unameinfo */
      return info.ticks;
    case _SC_OPEN_MAX:
      /* query via unameinfo */
      return info.max_open;
    case _SC_PAGESIZE:
      /* query via unameinfo */
      return psize;
    case _SC_RE_DUP_MAX:
      /* FIXME: read the regexp code */
      return 4;
    case _SC_STREAM_MAX:
      /* In theory down to RAM */
      return 256;
    case _SC_SYMLOOP_MAX:
      /* Not supported yet */
      return 1;
    case _SC_TTY_NAME_MAX:
      return 9;
    case _SC_TZNAME_MAX:
      return 6;
    /* Don't provide _SC_VERSION - we don't really meet POSIX! */
    case _SC_VERSION:
      return 0;
    case _SC_PHYS_PAGES:
      return info.memk/pscale;
    case _SC_AVPHYS_PAGES:
      return (info.memk-info.usedk)/pscale;
    case _SC_NPROCESSORS_CONF:
    case _SC_NPROCESSORS_ONLN:
      return 1;
    case _SC_FUZIX_LOADAVG1:
      return info.loadavg[0];
    case _SC_FUZIX_LOADAVG5:
      return info.loadavg[1];
    case _SC_FUZIX_LOADAVG15:
      return info.loadavg[2];
    default:
      errno = EINVAL;
      return -1;
  }
}
