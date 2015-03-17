#include <stdint.h>
#include <ctype.h>

#undef isalnum
int isalnum(int c)
{
	uint8_t bc = c;
	return isalpha(c) || ((bc >= '0') && (bc <= '9'));
}
