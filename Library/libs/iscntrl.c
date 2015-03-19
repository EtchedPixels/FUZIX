#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int iscntrl(int c)
{ return ((c >= 0) && (c <= 31)) || (c == 127); }

