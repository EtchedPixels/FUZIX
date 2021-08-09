/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * FIXME: helper for stdio needs to become re-entrant
 */

#include <string.h>

static char buf[34];


char * __ultostr(unsigned long val, int radix)
{
   register char *p;
   register int c;

   if( radix > 36 || radix < 2 ) return 0;

   p = buf+sizeof(buf);
   *--p = '\0';

   do
   {
      c = val%radix;
      val/=radix;
      if( c > 9 ) *--p = 'a'-10+c; else *--p = '0'+c;
   }
   while(val);
   return p;
}

char * __ltostr(long val, int radix)
{
   char *p;
   int flg = 0;
   if( val < 0 ) { flg++; val= -val; }
   p = __ultostr(val, radix);
   if(p && flg) *--p = '-';
   return p;
}

/* sadly we do not know the size of the user-provided buffer so we cannot write
   it backwards, so just copy the result from our own buffer whose size we know */

char *ultoa (unsigned long value, char *strP, int radix)
{
    return strcpy(strP, __ultostr(value, radix));
}

char *ltoa (long value, char *strP, int radix)
{
    return strcpy(strP, __ltostr(value, radix));
}
