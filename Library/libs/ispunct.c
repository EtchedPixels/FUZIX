#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int ispunct(int c)
{ return isascii(c) && !iscntrl(c) && !isalnum(c) && !isspace(c); }

