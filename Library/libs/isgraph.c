#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isgraph(int c)
{ return (c >= 33) && (c <= 126); }

