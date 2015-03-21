#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isalnum(int c)
{ return isdigit((uint8_t) c) || isalpha((uint8_t) c); }
