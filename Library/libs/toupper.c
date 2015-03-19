#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int toupper(int c)
{
	if (islower(c))
		c ^= 0x20;
	return c;
}

