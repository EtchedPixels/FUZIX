/**************************************************
  UZI (Unix Z80 Implementation) Utilities:  xfs1a.c
 ***************************************************/
/* Revisions:
 *  1.4.98 - Split xfs.c into parts for compilation with Hi-Tech C.   HFB
 */

 /*LINTLIBRARY*/
#include <stddef.h>
#include <stdint.h>
#include <strings.h>
#include "fuzix_fs.h"

int32_t _lseek(int16_t file, int32_t offset, int16_t flag)
{
	register inoptr ino;
	register int32_t retval;
	register int oftno;

	udata.u_error = 0;
	if ((ino = getinode(file)) == NULLINODE)
		return (-1);

	oftno = udata.u_files[file];

	retval = of_tab[oftno].o_ptr;

	switch (flag) {
	case 0:
		of_tab[oftno].o_ptr = offset;
		break;
	case 1:
		of_tab[oftno].o_ptr += offset;
		break;
	case 2:
		of_tab[oftno].o_ptr =
		    swizzle32(ino->c_node.i_size) + offset;
		break;
	default:
		udata.u_error = EINVAL;
		return (-1);
	}
	retval = of_tab[oftno].o_ptr;
	return retval;
}



void readi(inoptr ino)
{
	register uint16_t amount;
	register uint16_t toread;
	register blkno_t pblk;
	register char *bp;
	int dev;

	dev = ino->c_dev;
	switch (getmode(ino)) {

	case F_DIR:
	case F_REG:

		/* See of end of file will limit read */
		toread = udata.u_count =
		    min(udata.u_count,
			swizzle32(ino->c_node.i_size) - udata.u_offset);
		goto loop;

	case F_BDEV:
		toread = udata.u_count;
		dev = swizzle16(*(ino->c_node.i_addr));

	      loop:
		while (toread) {
			if ((pblk =
			     bmap(ino, udata.u_offset >> 9, 1)) != NULLBLK)
				bp = bread(dev, pblk, 0);
			else
				bp = zerobuf();

			bcopy(bp + (udata.u_offset & 511), udata.u_base,
			      (amount =
			       min(toread, 512 - (udata.u_offset & 511))));
			brelse((bufptr) bp);

			udata.u_base += amount;
			udata.u_offset += amount;
			toread -= amount;
		}
		break;

	default:
		udata.u_error = ENODEV;
	}
}



/* Writei (and readi) need more i/o error handling */

void writei(inoptr ino)
{
	register uint16_t amount;
	register uint16_t towrite;
	register char *bp;
	blkno_t pblk;
	int dev;

	dev = ino->c_dev;

	switch (getmode(ino)) {

	case F_BDEV:
		dev = swizzle16(*(ino->c_node.i_addr));
	case F_DIR:
	case F_REG:
		towrite = udata.u_count;
		goto loop;

	      loop:
		while (towrite) {
			amount =
			    min(towrite, 512 - (udata.u_offset & 511));

			if ((pblk =
			     bmap(ino, udata.u_offset >> 9, 0)) == NULLBLK)
				break;	/* No space to make more blocks */

			/* If we are writing an entire block, we don't care
			   about its previous contents */
			bp = bread(dev, pblk, (amount == 512));

			bcopy(udata.u_base, bp + (udata.u_offset & 511),
			      amount);
			bawrite((bufptr) bp);

			udata.u_base += amount;
			udata.u_offset += amount;
			towrite -= amount;
		}

		/* Update size if file grew */
		if (udata.u_offset > swizzle32(ino->c_node.i_size)) {
			ino->c_node.i_size = swizzle32(udata.u_offset);
			ino->c_dirty = 1;
		}
		break;

	default:
		udata.u_error = ENODEV;
	}
}



void updoff(int d)
{
	/* Update current file pointer */
	of_tab[udata.u_files[d]].o_ptr = udata.u_offset;
}
