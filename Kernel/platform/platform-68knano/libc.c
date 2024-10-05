#include <kernel.h>

/* FIXME: use proper fast methods */

void *memcpy(void *d, const void *s, size_t sz)
{
  unsigned char *dp = d;
  const unsigned char *sp = s;
  while(sz--)
    *dp++=*sp++;
  return d;
}

void *memcpy32(void *d, const void *s, size_t sz)
{
  uint32_t *dp = d;
  const uint32_t *sp = s;
  sz >>= 2;
  while(sz--)
    *dp++ = *sp++;
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

int memcmp(const void *a, const void *b, size_t n)
{
  const uint8_t *ap = a;
  const uint8_t *bp = b;
  while(n--) {
    if (*ap < *bp)
      return -1;
    if (*ap != *bp)
      return 1;
    ap++;
    bp++;
  }
  return 0;
}
