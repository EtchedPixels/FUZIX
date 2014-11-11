#include "cpu.h"

void *memcpy(void *d, void *s, size_t sz)
{
  unsigned char *dp, *sp;
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

/* Until we pull out the bits of libgcc that are useful instead */
void abort(void)
{
}

void *malloc(size_t size)
{
  return 0;
}