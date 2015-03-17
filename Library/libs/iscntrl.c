#include <stdint.h>
#include <ctype.h>

int iscntrl(int c)
{
	uint8_t bc = c;
	return ((bc >= 0) && (bc <= 31)) || (bc == 127);
}
