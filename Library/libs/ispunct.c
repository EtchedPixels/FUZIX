/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#endif

int ispunct(int c)
{
	return isascii(c) && !iscntrl(c) && !isalnum(c) && !isspace(c);
}

