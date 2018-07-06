/*	cfillbuf.c	1.6	83/05/13	*/

#include <stdio.h>
#include "cpm.h"

int c_fillbuf(C_FILE * fptr)
{

	int nsect;

	if (++fptr->c_blk == (use16bitptrs ? 8 : 16)) {
		if (fptr->c_dirp->blkcnt == (char) 0x80) {
			/* find next extent (if it exists) */
			if (getnext(fptr) == 0)
				return EOF;
		} else
			return EOF;
	}
	/* This seems to reccur - uninline ? */
	nsect = (fptr->c_seccnt > blksiz / seclth) ? blksiz / seclth : fptr->c_seccnt;
	if (nsect == 0)
		return EOF;
	fptr->c_seccnt -= nsect;
	if (getblock(blockno(fptr->c_blk), fptr->c_base, nsect) == EOF)
		return EOF;
	fptr->c_buf = fptr->c_base;
	fptr->c_cnt = nsect * seclth - 1;
	return (*fptr->c_buf++ & 0xff);
}
