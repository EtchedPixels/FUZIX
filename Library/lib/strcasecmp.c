/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>
#include <ctype.h>

int strcasecmp(const char *s, const char *d)
{
   for(;;)
   {
      if( *s != *d )
      {
	 if( tolower(*s) != tolower(*d) )
	    return *s - *d;
      }
      else if( *s == '\0' ) break;
      s++; d++;
   }
   return 0;
}

