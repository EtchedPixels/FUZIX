/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>
#include <ctype.h>

int strncasecmp(const char *s, const char *d, size_t l)
{
   while(l>0)
   {
      if( *s != *d )
      {
	 if( tolower(*s) != tolower(*d) )
	    return *s - *d;
      }
      else
	 if( *s == '\0' ) return 0;
      s++; d++; l--;
   }
   return 0;
}

