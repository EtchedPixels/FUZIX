/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

/* This uses strchr, strchr should be in assembler */

char *strpbrk(const char *str, const char *set)
{
  while (*str != '\0')
    if (strchr(set, *str) == 0)
      ++str;
    else
      return (char *) str;

  return 0;
}
