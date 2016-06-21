/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
#include <unistd.h>
#include <fcntl.h>

int creat(const char * file, mode_t mode)
{
  return open(file, O_TRUNC|O_CREAT|O_WRONLY, mode);
}
