#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int tolower(int c)
{
	if (isupper(c))
		c ^= 0x20;
	return c;
}

