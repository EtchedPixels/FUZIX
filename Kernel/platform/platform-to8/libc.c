#include <kernel.h>


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

void *memmove(void *d, const void *s, size_t sz)
{
  unsigned char *dp = d;
  const unsigned char *sp = s;
  if (sp >= dp) {
    while(sz--)
      *dp++=*sp++;
  } else {
    dp += sz;
    sp += sz;
    while(sz--)
      *--dp=*--sp;
  }
  return d;
}

void *memcpy(void *d, const void *s, size_t sz)
{
  return memmove(d,s,sz);
}
