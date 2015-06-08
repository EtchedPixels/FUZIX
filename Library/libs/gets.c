/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"
#include <errno.h>

char *gets(char *str)
{				/* BAD function; DON'T use it! */
	/* Auwlright it will work but of course _your_ program will crash */
	/* if it's given a too long line */
	register int c;
	register char *p = str;

	while (((c = getc(stdin)) != EOF) && (c != '\n'))
		*p++ = c;
	*p = '\0';
	return (((c == EOF) && (p == str)) ? NULL : str);	/* NULL == EOF */
}

char *gets_s(char *str, size_t maxlen)
{
	register int c;
	register char *p = str;
	char *end = p + maxlen - 1;
	uint8_t over = 0;

	/* NULL is defined as an error so we don't need to consume data */
	if (str == NULL || maxlen == 0)
		goto fail;

	while (((c = getc(stdin)) != EOF) && (c != '\n')) {
		/* < because we need space for the \0 */
		if (p < end)
			*p++ = c;
		else
			over = 1;
	}
	*p = '\0';

	/* NULL == EOF */
	if (!over)
		return (((c == EOF) && (p == str)) ? NULL : str);
fail:
	errno = ERANGE;
	return NULL;
}

int puts(const void *str)
{
	register int n;
	if (((n = fputs(str, stdout)) == EOF)
	    || (putc('\n', stdout) == EOF))
		return (EOF);
	return (++n);
}
