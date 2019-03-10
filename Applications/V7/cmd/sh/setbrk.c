/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	<string.h>

void *setbrk(intptr_t incr)
{
	uint8_t *a = sbrk(incr);
	if (a == (void *)-1) {
		write(2, "Out of memory.\n", 15);
		_exit(255);
	}
	brkend = a + incr;
	if (a != (uint8_t *)-1 && incr > 0)
		memset(a, 0, incr);
	return a;
}
