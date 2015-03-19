#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isblank(int c)
{ return (c == ' ') || (c == '\t'); }

