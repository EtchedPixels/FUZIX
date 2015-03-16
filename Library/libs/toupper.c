#include <ctype.h>

int toupper(int c)
{
	if (islower(c))
		c = c^0x20;
	return c;
}

