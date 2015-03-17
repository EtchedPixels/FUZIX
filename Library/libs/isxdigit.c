#include <stdint.h>
#include <ctype.h>

int isxdigit(int c)
{
	uint8_t bc = c;
	return isdigit(bc)
		|| ((bc >= 'a') && (bc <= 'f'))
		|| ((bc >= 'A') && (bc <= 'F'));
}
