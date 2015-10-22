#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>

/* Legacy username query: Do not use this function in new (or old ;-) ) code. */


static char cbuf[L_cuserid];

char *cuserid(char *buf)
{
  /* FIXME: need a getpwuid_r to do this threadsafe */
  struct passwd *p = getpwuid(getuid());
  if (p == NULL)
    return NULL;
  if (buf == NULL)
    buf = cbuf;
  strlcpy(buf, p->pw_name, L_cuserid);
  return buf;
}
