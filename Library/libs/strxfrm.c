#include <string.h>

size_t strxfrm(char *to, const char *from, size_t n)
{
  strncpy(to, from, n);
  return strnlen(to, n);
}
