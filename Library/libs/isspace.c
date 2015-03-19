#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isspace(int c)
{ return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ||
         (c == '\v') || (c == '\f'); }

