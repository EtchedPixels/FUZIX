/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
/* This is an implementation of the C standard IO package. */ 
    
#include "stdio-l.h"

void rewind(FILE * fp) 
{
	fseek(fp, 0L, SEEK_SET);
	clearerr(fp);
}

int fseek(FILE * fp, long offset, int ref) 
{
	/* if __MODE_READING and no ungetc ever done can just move pointer */ 
	/* This needs testing! */ 
	if ((fp->mode & (__MODE_READING | __MODE_UNGOT)) ==
		__MODE_READING && (ref == SEEK_SET || ref == SEEK_CUR)) {
		off_t fpos = lseek(fp->fd, 0L, SEEK_CUR);
		if (fpos == -1L)
			return EOF;
		if (ref == SEEK_CUR) {
			ref = SEEK_SET;
			offset += fpos + (fp->bufpos - fp->bufread);
		}
		if (ref == SEEK_SET) {
			if (offset < fpos && /* ??? */
				offset >= fpos + (fp->bufstart - fp->bufread)) {
				fp->bufpos = (int) (offset - fpos) + fp->bufread;
				return 0;
			}
		}
	}

	/* Use fflush to sync the pointers */ 
	if (fflush(fp) == EOF || lseek(fp->fd, offset, ref) < 0)
		return EOF;
	return 0;
}
