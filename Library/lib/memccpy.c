/*
 *	memccpy.c: copy memory until match or end marker
 */

#include <string.h>

void *memccpy(void *d, const void *s, int c, size_t n)
{
  char *s1 = d;
  const char *s2 = s;
  while (n) {
    n--;
    if ((*s1++ = *s2++) == c)
      break;
  }
  return d;
}