#include <stdint.h>
#include <ctype.h>

int islower(int c)
{
	uint8_t bc = c;
	return (bc >= 'a') && (bc <= 'z');
}
