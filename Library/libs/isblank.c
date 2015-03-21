/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#endif

int isblank(int c)
{
	uint8_t cb = c;
	return (cb == ' ') || (c == '\t');
}

