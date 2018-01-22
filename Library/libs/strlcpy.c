#include <string.h>

size_t strlcpy(char *dst, const char *src, size_t dstsize)
{
  size_t len = strnlen(src, dstsize);
  size_t cp = len >= dstsize ? dstsize - 1 : len;
  memcpy(dst, src, cp);
  dst[cp] = 0;
  return len;
}

size_t strlcat(char *dst, const char *src, size_t dstsize)
{
  size_t len = strlen(dst);
  /* No room at all: existing string fills the buffer */
  if (len >= dstsize - 1)
    return len + strlen(src);
  return strlcpy(dst + len, src, dstsize - len);
}

  