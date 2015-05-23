/*
 *	Internal wrapper (in theory) but used in various places it's helpful
 */
#include <stdlib.h>

/*********************** xitoa.c ***************************/

/* Don't go via long - the long version is expensive on an 8bit processor and
   can often be avoided */

static char buf[7];

char *_uitoa(unsigned int i)
{
	char *p = buf + sizeof(buf);
	int c;

	*--p = '\0';
	do {
		c = i % 10;
		i /= 10;
		*--p = '0' + c;
	} while(i);
	return p;
}

char *_itoa(int i) {
	char *p;
	if (i >= 0)
		return _uitoa(i);
	p = _uitoa(-i);
	*--p = '-';
	return p;
}
