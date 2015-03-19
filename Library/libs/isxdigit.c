#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isxdigit(int c)
{ return isdigit(c) || (((c|0x20) >= 'a') && ((c|0x20) <= 'f')); }

