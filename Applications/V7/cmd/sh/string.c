/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"


/* ========	general purpose string handling ======== */


STRING movstr(a, b)
register STRING a, b;
{
	while (*b++ = *a++);
	return (--b);
}

int any(c, s)
register CHAR c;
STRING s;
{
	register CHAR d;

	while (d = *s++) {
		if (d == c) {
			return (TRUE);
			;
		};
	}
	return (FALSE);
}

int cf(s1, s2)
register STRING s1, s2;
{
	while (*s1++ == *s2) {
		if (*s2++ == 0) {
			return (0);
			;
		};
	}
	return (*--s1 - *s2);
}

int length(as)
STRING as;
{
	register STRING s;

	if (s = as) {
		while (*s++);;
	}
	return (s - as);
}
