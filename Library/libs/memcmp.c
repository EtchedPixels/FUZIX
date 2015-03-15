/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * This file is licensed under the terms of the 3-clause BSD open source
 * license.
 */

#include	<string.h>

int
memcmp(const void *s1, const void *s2, size_t n)
{
	register const unsigned char *p1 = s1, *p2 = s2;

	if (n) {
		n++;
		while (--n > 0) {
			if (*p1++ == *p2++) continue;
			return *--p1 - *--p2;
		}
	}
	return 0;
}
