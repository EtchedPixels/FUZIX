/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <ctype.h>
#include <stdint.h>
#endif

int toupper(int c)
{
	uint8_t cb = c;
	if ((cb >= 'a') && (cb <= 'z'))
		cb ^= 0x20;
	return cb;
}

