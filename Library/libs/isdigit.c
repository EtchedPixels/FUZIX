#include <stdint.h>
#include <ctype.h>

#undef isdigit
int isdigit(int c)
{
	uint8_t bc = c;
	return (bc >= '0') && (bc <= '9');
}

