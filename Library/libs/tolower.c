#include <ctype.h>

int tolower(int c)
{
	if (isupper(c))
		c = c^0x20;
	return c;
}

