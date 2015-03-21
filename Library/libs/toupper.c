/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <ctype.h>
#endif

int toupper(int c)
{
	if (islower(c))
		c = c^0x20;
	return c;
}

