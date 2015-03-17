#include <stdint.h>
#include <ctype.h>

#undef isalpha
int isalpha(int c)
{
	uint8_t bc = c;
	bc = bc & 0x20;
	return (bc >= 'a') && (bc <= 'z');
}

