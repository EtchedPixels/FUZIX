#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <utmp.h>
#include <errno.h>

static char tmp[L_cuserid];

char *getlogin(void)
{
  if (getlogin_r(tmp, sizeof(tmp)))
    return NULL;
  return tmp;
}

int getlogin_r(char *buf, size_t len)
{
  char tty[UT_LINESIZE + 6];
  struct utmp ut, *utp;
  int err;
  int fd = open("/dev/tty", O_RDONLY|O_NOCTTY);

  if (fd == -1)
    return -1;
  
  if (len > UT_NAMESIZE + 1)
    len = UT_NAMESIZE + 1;

  err = ttyname_r(fd, buf, sizeof(tty));
  close(fd);
  if (err)
    return -1;
  /* Skip the /dev/ */
  strncpy(ut.ut_line, tty + 5, sizeof(ut.ut_line));
  ut.ut_type = USER_PROCESS;
  setutent();
  /* FIXME: move to getutline_r once we have it */
  utp = getutline(&ut);
  endutent();
  if (utp == NULL) {
    errno = ENOENT;
    return -1;
  }
  strlcpy(buf, utp->ut_user, len);
  return 0;
}
