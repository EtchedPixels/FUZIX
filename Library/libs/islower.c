#include <stdint.h>
#include <ctype.h>

#undef islower
int islower(int c)
{
	uint8_t bc = c;
	return (bc >= 'a') && (bc <= 'z');
}
