#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isupper(int c)
{ return (c >= 'A') && (c <= 'Z'); }

