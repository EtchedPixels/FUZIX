#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/* Writei (and readi) need more i/o error handling */
void readi(inoptr ino, uint8_t flag)
{
	usize_t amount;
	usize_t toread;
	blkno_t pblk;
	unsigned char *bp;
	uint16_t dev;
	bool ispipe;

	dev = ino->c_dev;
	ispipe = false;
	switch (getmode(ino)) {
	case F_DIR:
	case F_REG:

		/* See if end of file will limit read */

		udata.u_count = min(udata.u_count,
				ino->c_node.i_size - udata.u_offset);
		toread = udata.u_count;
		goto loop;

        case F_SOCK:
#ifdef CONFIG_NET
                if (is_netd())
                        return netd_sock_read(ino, flag);
#endif
	case F_PIPE:
		ispipe = true;
		while (ino->c_node.i_size == 0 && !(flag & O_NDELAY)) {
			if (ino->c_refs == 1)	/* No writers */
				break;
			/* Sleep if empty pipe */
			psleep(ino);
		}
		toread = udata.u_count = min(udata.u_count, ino->c_node.i_size);
		if (toread == 0) {
			udata.u_error = EWOULDBLOCK;
			break;
		}
		goto loop;

	case F_BDEV:
		toread = udata.u_count;
		dev = *(ino->c_node.i_addr);

	      loop:
		while (toread) {
			if ((pblk =
			     bmap(ino, udata.u_offset >> BLKSHIFT,
				  1)) != NULLBLK)
				bp = bread(dev, pblk, 0);
			else
				bp = zerobuf();

			amount =
			    min(toread, BLKSIZE - (udata.u_offset&BLKMASK));

			uputsys(bp + (udata.u_offset & BLKMASK), amount);

			brelse(bp);

			udata.u_base += amount;
			udata.u_offset += amount;
			if (ispipe && udata.u_offset >= 18 * BLKSIZE)
				udata.u_offset = 0;
			toread -= amount;
			if (ispipe) {
				ino->c_node.i_size -= amount;
				wakeup(ino);
			}
		}
		break;

	case F_CDEV:
		udata.u_count = cdread(ino->c_node.i_addr[0], flag);

		if (udata.u_count != -1)
			udata.u_offset += udata.u_count;
		break;

	default:
		udata.u_error = ENODEV;
	}
}



void writei(inoptr ino, uint8_t flag)
{
	usize_t amount;
	usize_t towrite;
	unsigned char *bp;
	bool ispipe;
	blkno_t pblk;
	uint16_t dev;
	bool ndelay = (flag & O_NDELAY) ? 1 : 0;

	dev = ino->c_dev;

	switch (getmode(ino)) {

	case F_BDEV:
		dev = *(ino->c_node.i_addr);
	case F_DIR:
	case F_REG:
		ispipe = false;
		towrite = udata.u_count;
		goto loop;

#ifdef CONFIG_NET
	case F_SOCK:
		if (!is_netd()) {
			udata.u_count = sock_write(ino, flag);
			break;
		}
#endif
	case F_PIPE:
		ispipe = true;
		/* FIXME: this will hang if you ever write > 16 * BLKSIZE
		   in one go - needs merging into the loop */
		while ((towrite = udata.u_count) > (16 * BLKSIZE) - 
					ino->c_node.i_size) {
			if (ino->c_refs == 1) {	/* No readers */
				udata.u_count = -1;
				udata.u_error = EPIPE;
				ssig(udata.u_ptab, SIGPIPE);
				return;
			}
			/* Sleep if empty pipe */
			if (ndelay) {
				udata.u_error = EWOULDBLOCK;
				return;
			}
			psleep(ino);
		}
		/* Sleep if empty pipe */
		goto loop;

	      loop:

		while (towrite) {
			amount = min(towrite, 512 - (udata.u_offset&BLKMASK));

			if ((pblk =
			     bmap(ino, udata.u_offset >> BLKSHIFT,
				  0)) == NULLBLK)
				break;	/* No space to make more blocks */

			/* If we are writing an entire block, we don't care
			 * about its previous contents
			 */
			bp = bread(dev, pblk, (amount == BLKSIZE));

			ugetsys(bp + (udata.u_offset & BLKMASK), amount);

			/* FIXME: O_SYNC */
			bawrite(bp);

			udata.u_base += amount;
			udata.u_offset += amount;
			if (ispipe) {
				if (udata.u_offset >= 18 * 512)
					udata.u_offset = 0;
				ino->c_node.i_size += amount;
				/* Wake up any readers */
				wakeup(ino);
			}
			towrite -= amount;
		}

		/* Update size if file grew */
		if (!ispipe) {
			if (udata.u_offset > ino->c_node.i_size) {
				ino->c_node.i_size = udata.u_offset;
				ino->c_flags |= CDIRTY;
			}
		}
		break;

	case F_CDEV:
		udata.u_count = cdwrite(ino->c_node.i_addr[0], flag);

		if (udata.u_count != -1)
			udata.u_offset += udata.u_count;
		break;
	default:
		udata.u_error = ENODEV;
	}
}

int16_t doclose(uint8_t uindex)
{
	int8_t oftindex;
	inoptr ino;

	if (!(ino = getinode(uindex)))
		return (-1);

	oftindex = udata.u_files[uindex];

	if (ino->c_refs == 1 && of_tab[oftindex].o_refs == 1) {
		if (isdevice(ino))
			d_close((int) (ino->c_node.i_addr[0]));
#ifdef CONFIG_NET
		if (issocket(ino)
			sock_close(ino);
#endif
	}
	udata.u_files[uindex] = NO_FILE;
	udata.u_cloexec &= ~(1 << uindex);
	oft_deref(oftindex);

	return (0);
}

inoptr rwsetup(bool is_read, uint8_t * flag)
{
	inoptr ino;
	struct oft *oftp;

	udata.u_sysio = false;	/* I/O to user data space */
	udata.u_base = (char *) udata.u_argn1;	/* buf */
	udata.u_count = (susize_t) udata.u_argn2;	/* nbytes */

	if ((ino = getinode(udata.u_argn)) == NULLINODE) {
		/* kprintf("[WRS: rwsetup(): getinode(%x) fails]", udata.u_argn); */
		return (NULLINODE);
	}

	oftp = of_tab + udata.u_files[udata.u_argn];
	*flag = oftp->o_access;
	if (O_ACCMODE(oftp->o_access) == (is_read ? O_WRONLY : O_RDONLY)) {
		udata.u_error = EBADF;
		return (NULLINODE);
	}
	setftime(ino, is_read ? A_TIME : (A_TIME | M_TIME | C_TIME));

	if (getmode(ino) == F_REG && is_read == 0
	    && (oftp->o_access & O_APPEND))
		oftp->o_ptr = ino->c_node.i_size;
	/* Initialize u_offset from file pointer */
	udata.u_offset = oftp->o_ptr;
	/* FIXME: for 32bit we will need to check for overflow of the
           file size here in the r/w inode code */
	return (ino);
}
