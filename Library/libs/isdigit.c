/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#endif

int isdigit(int c)
{
	return ((uint8_t)c >= '0') && ((uint8_t)c <= '9');
}

