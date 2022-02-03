/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

/* We've now got a nice fast strchr and memcmp use them */

char *strstr(const char *s1, const char *s2)
{
   int l = strlen(s2);
   char *p = (char *)s1;

   if (l == 0)
      return p;

   while ((p = strchr(p, *s2)) != NULL)
   {
      if (memcmp(p, s2, l) == 0)
         return p;
      p++;
   }
   return NULL;
}

