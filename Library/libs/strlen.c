/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * This file is licensed under the terms of the 3-clause BSD open source
 * license.
 */

#include	<string.h>

size_t
strlen(const char *org)
{
	register const char *s = org;

	while (*s++)
		/* EMPTY */ ;

	return --s - org;
}
