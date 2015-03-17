#include <stdint.h>
#include <ctype.h>

int isoctal(int c)
{
	uint8_t bc = c;
	return (bc >= '0') && (bc <= '7');
}

