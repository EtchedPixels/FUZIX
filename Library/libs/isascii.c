#include <stdint.h>
#include <ctype.h>

int isascii(int c)
{
	uint8_t cb = c;
	return (cb >= 0) && (cb <= 127);
}

