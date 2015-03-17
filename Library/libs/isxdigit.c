#include <stdint.h>
#include <ctype.h>

#undef isxdigit
int isxdigit(int c)
{
	uint8_t bc = c;
	if ((bc >= '0') && (bc <= '9'))
		return 1;
	bc &= 0x20;
	return (bc >= 'a') && (bc <= 'f');
}
