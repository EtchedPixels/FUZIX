/*
 *	An implementation of atoi avoiding longs and 8bit unfriendly
 *	math
 */
#include <stdlib.h>
#include <ctype.h>

/**************************** atoi.c ****************************/
int atoi(const char *str)
{
	int r = 0;
	uint8_t minus = 0;

	while(isspace(*str))
		str++;
	if (*str == '-') {
		minus = 1;
		str++;
	}
	while(*str) {
		uint8_t c = *str++;
		c -= '0';
		if (c > 9)
			break;
		r = ((r << 2) + r) << 1;
		r += c;
	}
	return minus ? - r : r;
}
