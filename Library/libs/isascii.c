#include <ctype.h>

int isascii(int c)
{
	return (c >= 0) && (c <= 127);
}

