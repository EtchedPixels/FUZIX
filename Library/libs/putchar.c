#include <stdio.h>

/* Some compilers will emit calls to this function, even though it's strictly
 * a macro; so we have to have a real function version of it. */

int (putchar)(int c)
{
	return putchar(c);
}

