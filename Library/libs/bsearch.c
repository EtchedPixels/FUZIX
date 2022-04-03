/*
 * This file lifted in toto from 'Dlibs' on the atari ST  (RdeBath)
 *
 * 
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *  "It's not reality that's important, but how you perceive things."
 */
#include <stdlib.h>

static int _bsearch;		/* index of element found, or where to
				 * insert */

void *bsearch(void *_key, void *_base, size_t num, size_t size,
	      cmp_func_t cmp)
{
	int a, b, c, dir;
	char *key = _key;
	char *base = _base;

	a = 0;
	b = num - 1;
	while (a <= b) {
		c = (a + b) >> 1;	/* == ((a + b) / 2) */
		if ((dir = (*cmp) ((base + (c * size)), key)) != 0) {
			if (dir > 0)
				b = c - 1;
			else	/* (dir < 0) */
				a = c + 1;
		} else {
			_bsearch = c;
			return (base + (c * size));
		}
	}
	_bsearch = b;
	return (NULL);
}
