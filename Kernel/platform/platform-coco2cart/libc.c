#include <kernel.h>

void *memcpy(void *d, const void *s, size_t sz)
{
  unsigned char *dp = d;
  const unsigned char *sp = s;
  while(sz--)
    *dp++=*sp++;
  return d;
}

void *memset(void *d, int c, size_t sz)
{
  unsigned char *p = d;
  while(sz--)
    *p++ = c;
  return d;
}

size_t strlen(const char *p)
{
  const char *e = p;
  while(*e++);
  return e-p-1;
}
