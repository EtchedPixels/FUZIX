#include "cpu.h"

void *memcpy(void *d, void *s, size_t sz)
{
  unsigned char *dp, *sp;
  while(sz--)
    *dp++=*sp++;
  return d;
}

size_t strlen(const char *p)
{
  const char *e = p;
  while(*e++);
  return e-p-1;
}

