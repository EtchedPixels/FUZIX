#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

void updoff(void)
{
	/* Update current file pointer */
	of_tab[udata.u_files[udata.u_argn]].o_ptr = udata.u_offset;
}


/*******************************************
  seek (file, offset, flag)         Function 9        ?
  int16_t file;
  uint32_t *offset;
  int16_t flag;
 ********************************************/
#define file   (uint16_t)udata.u_argn
#define offset (uint32_t *)udata.u_argn1
#define flag   (int16_t)udata.u_argn2

/* We copy the 32bit offset in and out rather than passing it
   as a 32bit OS might */
int16_t _lseek(void)
{
	inoptr ino;
	struct oft *o;
	off_t p;
	off_t n;
	
	if (uget(offset, &n, sizeof(n)))
	        return -1;

	if ((ino = getinode(file)) == NULLINODE)
		return (-1);

	if (getmode(ino) == F_PIPE) {
		udata.u_error = ESPIPE;
		return (-1);
	}

	o = &of_tab[udata.u_files[file]];
	p = o->o_ptr;

	switch (flag) {
	case 0:
		p = n;
		break;
	case 1:
		p += n;
		break;
	case 2:
		p = ino->c_node.i_size + n;
		break;
	default:
                goto bad;
	}
	if (p < 0)
	        goto bad;
        o->o_ptr = p;
	uput(&p, offset, sizeof(n));
	return 0;
bad:
	udata.u_error = EINVAL;
	return (-1);

}

#undef file
#undef offset
#undef flag



/*******************************************
  sync()
 ********************************************/

int16_t _sync(void)
{
	inoptr ino;

	/* Write out modified inodes */

	for (ino = i_tab; ino < i_tab + ITABSIZE; ++ino)
		if (ino->c_refs > 0 && (ino->c_flags & CDIRTY)) {
			wr_inode(ino);
			ino->c_flags &= ~CDIRTY;
		}

        /* This now also indirectly does the superblocks as they
           are buffers that are pinned */
	bufsync();		/* Clear buffer pool */

	return 0;
}




/*******************************************
  stat (path, buf)                 Function 15
  char *path;
  char *buf;
 ********************************************/
#define path (char *)udata.u_argn
#define buf (char *)udata.u_argn1

int16_t _stat(void)
{
	inoptr ino;
	if (!(ino = n_open(path, NULLINOPTR)))
		return (-1);
	stcpy(ino, buf);
	i_deref(ino);
	return (0);
}

#undef path
#undef buf


/*******************************************
  fstat (fd, buf)                  Function 16
  int16_t fd;
  char *buf;
 ********************************************/
#define fd (int16_t)udata.u_argn
#define buf (char *)udata.u_argn1

int16_t _fstat(void)
{
	inoptr ino;

	if ((ino = getinode(fd)) == NULLINODE)
		return (-1);

	return stcpy(ino, buf);
}

#undef fd
#undef buf


/* Utility for stat and fstat */
int stcpy(inoptr ino, char *buf)
{
	int err = uput((char *) &(ino->c_dev), buf, 12);
	err |= uput((char *) &(ino->c_node.i_addr[0]), buf + 12, 2);
	err |= uput((char *) &(ino->c_node.i_size), buf + 14, 16);
	return err;
}


int16_t dup_op(int fd, int base)
{
	int8_t newd;

	if ((newd = uf_alloc_n(base)) == -1)
		return (-1);

	udata.u_files[newd] = udata.u_files[fd];
	++of_tab[udata.u_files[fd]].o_refs;

	return (newd);
}

/*******************************************
  dup (oldd)                       Function 17
  int16_t oldd;
 ********************************************/
#define oldd (uint16_t)udata.u_argn

int16_t _dup(void)
{
	int8_t newd;
	if (getinode(oldd) == NULLINODE)
		return (-1);
	if ((newd = uf_alloc()) == -1)
		return (-1);

	udata.u_files[newd] = udata.u_files[oldd];
	++of_tab[udata.u_files[oldd]].o_refs;

	return (newd);
}

#undef oldd


/*******************************************
  dup2 (oldd, newd)                Function 36        ?
  int16_t oldd;
  int16_t newd;
 ********************************************/
#define oldd (int16_t)udata.u_argn
#define newd (int16_t)udata.u_argn1

int16_t _dup2(void)
{

	if (getinode(oldd) == NULLINODE)
		return (-1);

	if (newd < 0 || newd >= UFTSIZE) {
		udata.u_error = EBADF;
		return (-1);
	}

	if (udata.u_files[newd] != NO_FILE)
		doclose(newd);

	udata.u_files[newd] = udata.u_files[oldd];
	++of_tab[udata.u_files[oldd]].o_refs;

	return (0);
}

#undef oldd
#undef newd


/*******************************************
  umask (mask)                     Function 21        ?
  int mask;
 ********************************************/
#define mask (int16_t)udata.u_argn

int16_t _umask(void)
{
	int omask;

	omask = udata.u_mask;
	udata.u_mask = mask & 0777;
	return (omask);
}

#undef mask



/*******************************************
  ioctl (fd, request, data)        Function 29
  int  fd;
  int  request;
  char *data;
 ********************************************/
#define fd (int)udata.u_argn
#define request (int)udata.u_argn1
#define data (char *)udata.u_argn2

int16_t _ioctl(void)
{
	inoptr ino;
	uint16_t dev;

	if ((ino = getinode(fd)) == NULLINODE)
		return (-1);

	if (!(isdevice(ino))) {
		udata.u_error = ENOTTY;
		return (-1);
	}

	if (!(getperm(ino) & OTH_WR)) {
		udata.u_error = EPERM;
		return (-1);
	}

	dev = ino->c_node.i_addr[0];

	/* top bit of request is reserved for kernel originated magic */
	if (d_ioctl(dev, request & 0x7FFF, data))
		return (-1);
	return (0);
}

#undef fd
#undef request
#undef data



/*******************************************
close (uindex)                    Function 2
int16_t uindex;
********************************************/
#define uindex (int8_t)udata.u_argn

int16_t _close(void)
{
	return (doclose(uindex));
}

#undef uindex


/*******************************************
pipe (fildes)                    Function 40		?
int fildes[];
********************************************/
#define fildes (int16_t *)udata.u_argn

int16_t _pipe(void)
{
	int8_t u1, u2, oft1, oft2;
	inoptr ino;

/* bug fix SN */
	if ((u1 = uf_alloc()) == -1)
		goto nogood;
	if ((oft1 = oft_alloc()) == -1)
		goto nogood;
	udata.u_files[u1] = oft1;

	if ((u2 = uf_alloc()) == -1)
		goto nogood2;
	if ((oft2 = oft_alloc()) == -1)
		goto nogood2;

	if (!(ino = i_open(root_dev, 0))) {
		oft_deref(oft2);
		goto nogood2;
	}

	udata.u_files[u2] = oft2;

	of_tab[oft1].o_ptr = 0;
	of_tab[oft1].o_inode = ino;
	of_tab[oft1].o_access = O_RDONLY;

	of_tab[oft2].o_ptr = 0;
	of_tab[oft2].o_inode = ino;
	of_tab[oft2].o_access = O_WRONLY;

	++ino->c_refs;
	ino->c_node.i_mode = F_PIPE | 0777;	/* No permissions necessary on pipes */
	ino->c_node.i_nlink = 0;	/* a pipe is not in any directory */

	// write results to userspace
	uputw(u1, fildes);
	uputw(u2, fildes + 1);
	return (0);

      nogood2:
	oft_deref(oft1);
	udata.u_files[u1] = NO_FILE;
      nogood:
	return (-1);
}

#undef fildes


/*******************************************
unlink (path)                     Function 6
char *path;
********************************************/
#define path (char *)udata.u_argn

/* Helper for the bits shared with rename */
int16_t unlinki(inoptr ino, inoptr pino, char *fname)
{
	if (getmode(ino) == F_DIR) {
		udata.u_error = EISDIR;
		return -1;
	}

	/* Remove the directory entry (ch_link checks perms) */
	if (!ch_link(pino, fname, "", NULLINODE))
		return -1;

	/* Decrease the link count of the inode */
	if (!(ino->c_node.i_nlink--)) {
		ino->c_node.i_nlink += 2;
		kprintf("_unlink: bad nlink\n");
	}
	setftime(ino, C_TIME);
	return (0);
}

/* kind of pinned here by unlinki() */
int16_t _unlink(void)
{
	inoptr ino;
	inoptr pino;
	int r;
	char fname[FILENAME_LEN + 1];

	ino = n_open(path, &pino);

	if (!(pino && ino)) {
		if (pino)	/* parent exist */
			i_deref(pino);
		udata.u_error = ENOENT;
		return (-1);
	}
	filename(path, fname);
	r = unlinki(ino, pino, fname);
	i_deref(pino);
	i_deref(ino);
	return r;
}

#undef path



/*******************************************
read (d, buf, nbytes)             Function 7
int16_t  d;
char   *buf;
uint16_t nbytes;
********************************************/
#define d (int16_t)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes (susize_t)udata.u_argn2

int16_t _read(void)
{
	inoptr ino;
	uint8_t flag;

	if (!valaddr(buf, nbytes))
	        return -1;
	/* Set up u_base, u_offset, ino; check permissions, file num. */
	if ((ino = rwsetup(true, &flag)) == NULLINODE)
		return -1;	/* bomb out if error */

	readi(ino, flag);
	updoff();

	return (udata.u_count);
}

#undef d
#undef buf
#undef nbytes

/*******************************************
getdirent (d, buf, nbytes)       Function 59
int16_t  d;
char   *buf;
uint16_t nbytes;
********************************************/
#define d (int16_t)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes udata.u_argn2

/*
 *	Trivial helper to isolate directory size changes
 *	if we make them.
 */
int16_t _getdirent(void)
{
        if (nbytes < 32) {
                udata.u_error = ENOSPC;
                return -1;
        }
        nbytes = DIR_LEN;
        return _read();
}

#undef d
#undef buf
#undef nbytes


/*******************************************
write (d, buf, nbytes)            Function 8
int16_t  d;
char   *buf;
uint16_t nbytes;
********************************************/
#define d (int16_t)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes (susize_t)udata.u_argn2

int16_t _write(void)
{
	inoptr ino;
	uint8_t flag;

	if (!valaddr(buf, nbytes))
	        return -1;
	/* Set up u_base, u_offset, ino; check permissions, file num. */
	if ((ino = rwsetup(false, &flag)) == NULLINODE)
		return (-1);	/* bomb out if error */

	writei(ino, flag);
	updoff();

	return (udata.u_count);
}

#undef d
#undef buf
#undef nbytes

