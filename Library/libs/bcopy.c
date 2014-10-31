/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Fixed to use memmove - AC 2014
 */
#include <string.h>
#include <sys/types.h>

void bcopy(const void *src, void *dest, size_t len)
{
   (void)memmove(dest, src, len);
}
