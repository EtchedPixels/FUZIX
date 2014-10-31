/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"

int fclose(FILE * fp)
{
	int rv = 0;

	if (fp == 0) {
		errno = EINVAL;
		return EOF;
	}
	if (fflush(fp))
		return EOF;
	if (close(fp->fd))
		rv = EOF;
	fp->fd = -1;
	if (fp->mode & __MODE_FREEBUF) {
		free(fp->bufstart);
		fp->mode &= ~__MODE_FREEBUF;
		fp->bufstart = fp->bufend = 0;
	}
	if (fp->mode & __MODE_FREEFIL) {
		FILE *ptr = __IO_list, *prev = 0;

		fp->mode = 0;
		while (ptr && ptr != fp)
			ptr = ptr->next;
		if (ptr == fp) {
			if (prev == 0)
				__IO_list = fp->next;
			else
				prev->next = fp->next;
		}
		free(fp);
	} else
		fp->mode = 0;
	return rv;
}
