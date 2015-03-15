#include "cpu.h"

void* __fastcall__ memcpy(void *d, const void *s, size_t sz)
{
  uint8_t *dp = d;
  uint8_t *sp = s;
  while(sz--)
    *dp++=*sp++;
  return d;
}

size_t __fastcall__ strlen(const char *p)
{
  const char *e = p;
  while(*e++);
  return e-p-1;
}

