/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"

int fgetc(FILE * fp)
{
	int ch;

	if (fp->mode & __MODE_WRITING)
		fflush(fp);
	/* Can't read or there's been an EOF or error then return EOF */
	if ((fp->mode & (__MODE_READ | __MODE_EOF | __MODE_ERR)) !=
	    __MODE_READ)
		return EOF;
	/* Nothing in the buffer - fill it up */
	if (fp->bufpos >= fp->bufread) {
		fp->bufpos = fp->bufread = fp->bufstart;
		ch = fread(fp->bufpos, 1, fp->bufend - fp->bufstart, fp);
		if (ch == 0)
			return EOF;
		fp->bufread += ch;
		fp->mode |= __MODE_READING;
		fp->mode &= ~__MODE_UNGOT;
	}
	ch = *(fp->bufpos++);
	return ch;
}
