/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#undef isascii
#endif

int isascii(int c)
{
	return !((uint8_t)c & 0x80);
}

