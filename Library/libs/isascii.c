#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isascii(int c)
{ return (c >= 0) && (c <= 127); }

