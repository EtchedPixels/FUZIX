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


char *movstr(register const char *a, register char *b)
{
	while ( (*b++ = *a++) );
	return (--b);
}

int any(char c, const char *s)
{
	register char d;

	while ( (d = *s++) ) {
		if (d == c) {
			return (TRUE);
			;
		};
	}
	return (FALSE);
}

int cf(register const char *s1, register const char *s2)
{
	while (*s1++ == *s2) {
		if (*s2++ == 0) {
			return (0);
			;
		};
	}
	return (*--s1 - *s2);
}

int length(const char *as)
{
	register const char *s;

	if ( (s = as) ) {
		while (*s++);;
	}
	return (s - as);
}
