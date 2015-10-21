#include <unistd.h>

void swab(const void *from, void *to, ssize_t n)
{
  const uint8_t *f = from;
  uint8_t *t = to;
  n >>= 1;
  while(n--) {
    *t++ = f[1];
    *t++ = *f;
    f += 2;
  }
}
