/*	blockio.c	1.4	83/05/13	*/

#include <stdio.h>
#include <stdint.h>
#include "cpm.h"

/*
 * Get a full block or a part of a block from floppy disk file
 * nsect gives the actual number of physical sectors to read.
 * if nsect is negative then always read a full block.
 */

int getblock(unsigned int blockno, char *buffer, int nsect)
{

	unsigned int sect, track, counter;

#ifdef DEBUG
	printf("block: %d\n", blockno);
#endif
	if (nsect < 0)
		nsect = blksiz / seclth;

	/* Translate block number into logical track/sector address */
	sect = (blockno * (blksiz / seclth) + sectrk * restrk) % sectrk + 1;
	track = (blockno * (blksiz / seclth) + sectrk * restrk) / sectrk;
	/* read the block */
	for (counter = 0; counter < nsect; counter++) {
		if (getpsect(track, skewtab[sect++ - 1], buffer + (seclth * counter))
		    == EOF)
			return EOF;
		if (sect > sectrk) {
			sect = 1;
			track++;
		}
	}
	return 0;
}

/*
 * Save a full block or a part of a block in floppy disk file
 * If nsects is negative, write a full block.
 */

int putblock(unsigned int blockno, char *buffer, int nsect)
{

	unsigned int sect, track, counter;

	if (nsect < 0)
		nsect = blksiz / seclth;

	/* Translate block number into logical track/sector address */
	sect = (blockno * (blksiz / seclth) + sectrk * restrk) % sectrk + 1;
	track = (blockno * (blksiz / seclth) + sectrk * restrk) / sectrk;
	/* write the block */
	for (counter = 0; counter < nsect; counter++) {
		if (putpsect(track, skewtab[sect++ - 1], buffer + (seclth * counter))
		    == EOF)
			return EOF;
		if (sect > sectrk) {
			sect = 1;
			track++;
		}
	}
	return 0;
}
