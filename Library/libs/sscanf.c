/*
 * This file based on scanf.c from 'Dlibs' on the atari ST  (RdeBath)
 *
 * 19-OCT-88: Dale Schumacher
 * > John Stanley has again been a great help in debugging, particularly
 * > with the printf/scanf functions which are his creation.  
 *
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *  "It's not reality that's important, but how you perceive things."
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

int sscanf(const char * sp, const char * fmt, ...)
{
static FILE  string[1] =
{
   {0, (char*)(unsigned) -1, 0, 0, (char*) (unsigned) -1, -1,
    _IOFBF | __MODE_READ}
};

  va_list ptr;
  int rv;
  unsigned char *p = string->bufpos;

  va_start(ptr, fmt);
  string->bufpos = (unsigned char *)sp;
  rv = vfscanf(string,fmt,ptr);
  va_end(ptr);

  string->bufpos = p;
  return rv;
}
