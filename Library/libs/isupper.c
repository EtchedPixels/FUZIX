#include <stdint.h>
#include <ctype.h>

#undef isupper
int isupper(int c)
{
	uint8_t bc = c;
	return (bc >= 'A') && (bc <= 'Z');
}
