/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * Copyright (C) 2015 Erkin Alp Güney <erkinalp9035@gmail.com>
 * This file is based on the work from Linux-8086 C library and
 * is distributed under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <types.h>

extern ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
	register int c;
	register char *p = *lineptr;
	if (!(*lineptr)) *lineptr=malloc(64); /* Minimal size possible */
	while ((c = getc(stream)) != EOF) {
		*p++ = c;
		if (c == delim) break;
	}
	*p = '\0';
	*n = p - (*lineptr);
	if (realloc(*lineptr,*n)) return *n; else return -1;
}

extern ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
	return getdelim(lineptr,n,'\n',stream);
}
