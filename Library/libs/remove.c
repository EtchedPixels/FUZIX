/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int remove(const char *src)
{
   int er = errno;
   int rv = unlink(src);
   if( rv < 0 && errno == EISDIR )
      rv = rmdir(src);
   if( rv >= 0 ) errno = er;
   return rv;
}
