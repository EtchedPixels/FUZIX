#include <stdint.h>
#include <ctype.h>

int isupper(int c)
{
	uint8_t bc = c;
	return (bc >= 'A') && (bc <= 'Z');
}
