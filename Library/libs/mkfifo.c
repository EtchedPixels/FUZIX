/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <sys/types.h>
#include <unistd.h>

int mkfifo(const char *path, mode_t mode)
{
   return mknod(path, mode | S_IFIFO, 0);
}