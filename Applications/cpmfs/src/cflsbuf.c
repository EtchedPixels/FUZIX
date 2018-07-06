/*	cflsbuf.c	1.10	83/05/13	*/

#include <stdio.h>
#include "cpm.h"

/*
 * Flush a full block to floppy disk file
 * (these routines are never called unless a full block is
 * to be written)
 * Create a new extent if required
 */

int c_flush(C_FILE * fptr)
{

	int it;

	if (!(fptr->c_flag & WRITE)) {
		fprintf(stderr, "no write access");
		return EOF;
	}
	fptr->c_seccnt += blksiz / seclth;
	/* caution: blockno() might evaluate its argument twice */
	if (putblock(blockno(fptr->c_blk), fptr->c_base, -1) == EOF)
		return EOF;
	fptr->c_blk++;
	if (fptr->c_blk == (use16bitptrs ? 8 : 16)) {
		fptr->c_dirp->blkcnt = (char) 0x80;
		savedir();
		/* create new extent */
		if ((it = creext(fptr->c_ext)) == 0) {
			fprintf(stderr, "can't create new extent, current: %d\n", fptr->c_ext);
			return EOF;
		}
		fptr->c_dirp = dirbuf + it;
		fptr->c_ext = it;
		fptr->c_blk = 0;
		fptr->c_seccnt = 0;
		fptr->c_extno++;
		fptr->c_dirp->extno = fptr->c_extno;
	}
	fptr->c_buf = fptr->c_base;
	fptr->c_cnt = blksiz;
	if ((it = alloc()) == '\0') {
		fprintf(stderr, "disk full\n");
		return EOF;
	}
	if (use16bitptrs) {
		fptr->c_dirp->pointers[2 * fptr->c_blk] = it & 0xff;
		fptr->c_dirp->pointers[2 * fptr->c_blk + 1] = (it >> 8) & 0xff;
	} else
		fptr->c_dirp->pointers[fptr->c_blk] = it;
	return 0;
}

int c_flsbuf(int c, C_FILE * fptr)
{
	if (c_flush(fptr) == EOF)
		return EOF;
	*(fptr->c_buf++) = c;
	fptr->c_cnt--;
	return c;
}

/*
 * move the contents of 'buf' into the cpm block buffer,
 * flush the buffer if full (for binary file transfers)
 */

int c_write(C_FILE * fptr, char *buf, int cnt)
{
	int i = cnt;

	while (i-- > 0) {
		*(fptr->c_buf++) = *(buf++);
		if (--fptr->c_cnt == 0)
			if (c_flush(fptr) == EOF)
				return EOF;
	}
	return cnt;
}
