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
	brkend = a + incr;
	if (a != (uint8_t *)-1 && incr > 0)
		memset(a, 0, incr);
	return a;
}
