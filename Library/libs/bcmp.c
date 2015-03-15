/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
#include <string.h>
#include <sys/types.h>

int bcmp(const void *dest, const void *src, size_t len)
{
   return memcmp(dest, src, len);
}
