/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#endif

int isascii(int c)
{
	uint8_t cb = c;
	return (cb >= 0) && (cb <= 127);
}

