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
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

int vsscanf(const char * sp, const char *fmt, va_list ap)
{
static FILE  string[1] =
{
   {0, (char*)(unsigned) -1, 0, 0, (char*) (unsigned) -1, -1,
    _IOFBF | __MODE_READ}
};

  string->bufpos = (unsigned char *)sp;
  return vfscanf(string,fmt,ap);
}
