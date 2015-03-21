/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#endif

int isxdigit(int c)
{
	uint8_t bc = c;
	if (isdigit(bc))
		return 1;
	bc |= 0x20;
	return ((bc >= 'a') && (bc <= 'f'));
}
