#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isalpha(int c)
{ return ((c|0x20) >= 'a') && ((c|0x20) <= 'z'); }

