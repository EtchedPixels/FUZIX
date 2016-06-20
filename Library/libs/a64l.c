#include <stdio.h>

long a64l(const char *s)
{
  uint32_t n = 0;
  const char *se = s + 6;
  while(*s && s < se) {
    n <<= 6;
    if (*s == '.')
      n++;
    if (*s >= '0' && *s <= '9')
      n += *s - '0' + 2;
    else if (*s >= 'A' && *s <= 'Z')
      n += *s - 'A' + 12;
    else
      n += *s - 'a' + 38;
    s++;
  }
  return n;
}
