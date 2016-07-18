#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>


/*******************************************
open (name, flag, mode)           Function 1
char *name;
int16_t flag;
int16_t mode;
********************************************/
#define name (char *)udata.u_argn
#define flag (uint16_t)udata.u_argn1
#define mode (uint16_t)udata.u_argn2

arg_t _open(void)
{
	int8_t uindex;
	int8_t oftindex;
	staticfast inoptr ino;
	int16_t perm;
	staticfast inoptr parent;
	char fname[FILENAME_LEN + 1];
	int trunc;
	int r;
	int w;
	int j;

	parent = NULLINODE;

	trunc = flag & O_TRUNC;
	r = (flag + 1) & 1;
	w = (flag + 1) & 2;

	if (O_ACCMODE(flag) == 3 || (flag & O_BADBITS)) {
		udata.u_error = EINVAL;
		return (-1);
	}
	if ((uindex = uf_alloc()) == -1)
		return (-1);

	if ((oftindex = oft_alloc()) == -1)
		goto nooft;

	ino = n_open(name, &parent);
	if (ino) {
		i_deref(parent);
		/* O_EXCL test */
		if ((flag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
			udata.u_error = EEXIST;
			goto idrop;
		}
	} else {
		/* The n_open failed */
		if (udata.u_error == EFAULT)
			goto cantopen;

		/* New file */
		if (!(flag & O_CREAT)) {
			udata.u_error = ENOENT;
			goto cantopen;
		}
		filename(name, fname);

		/* newfile drops parent for us */
		ino = newfile(parent, fname);
		if (!ino) {
			/* on error, newfile sets udata.u_error */
			goto cantopen;
		}

		/* new inode, successfully created */
		ino->c_node.i_mode =
		    (F_REG | (mode & MODE_MASK & ~udata.u_mask));
		setftime(ino, A_TIME | M_TIME | C_TIME);
		wr_inode(ino);
	}

	/* Book our slot in case we block opening a device */
	of_tab[oftindex].o_inode = ino;

	perm = getperm(ino);
	if ((r && !(perm & OTH_RD)) || (w && !(perm & OTH_WR))) {
		udata.u_error = EACCES;
		goto cantopen;
	}
	if (w) {
		if (getmode(ino) == MODE_R(F_DIR)) {
			udata.u_error = EISDIR;
			goto cantopen;
		}
		/* Special case - devices on a read only file system may
		   be opened read/write */
		if (!isdevice(ino) && (ino->c_flags & CRDONLY)) {
			udata.u_error = EROFS;
			goto cantopen;
		}
	}

	if (isdevice(ino)) {
		inoptr *iptr = &of_tab[oftindex].o_inode;
		/* d_open may block and thus ino may become invalid as may
		   parent (but we don't need it again). It may also be changed
		   by the call to dev_openi */

		if (dev_openi(iptr, flag) != 0)
			goto cantopen;

		/* May have changed */
		/* get the static pointer back in case it changed via dev 
		   usage or just because we blocked */
		ino = *iptr;
	}

	if (trunc && getmode(ino) == MODE_R(F_REG)) {
		if (f_trunc(ino))
			goto idrop;
		for (j = 0; j < OFTSIZE; ++j)
			/* Arguably should fix at read/write */
			if (of_tab[j].o_inode == ino)
				of_tab[j].o_ptr = 0;
	}

	udata.u_files[uindex] = oftindex;

	of_tab[oftindex].o_ptr = 0;
	of_tab[oftindex].o_access = flag;	/* Save the low bits only */
	if (flag & O_CLOEXEC)
		udata.u_cloexec |= (1 << oftindex);
	/* FIXME: ATIME ? */

/*
 *         Sleep process if no writer or reader
 */
	if (getmode(ino) == MODE_R(F_PIPE) && of_tab[oftindex].o_refs == 1
	    && !(flag & O_NDELAY))
		psleep(ino);

        /* From the moment of the psleep ino is invalid */

	return (uindex);
      idrop:
	i_deref(ino);
      cantopen:
	oft_deref(oftindex);	/* This will call i_deref() */
      nooft:
	udata.u_files[uindex] = NO_FILE;
	return (-1);
}

#undef name
#undef flag
#undef mode



/*******************************************
link (name1, name2)               Function 5
char *name1;
char *name2;
********************************************/
#define name1 (char *)udata.u_argn
#define name2 (char *)udata.u_argn1

arg_t _link(void)
{
	inoptr ino;
	inoptr ino2;
	inoptr parent2;
	char fname[FILENAME_LEN + 1];

	if (!(ino = n_open(name1, NULLINOPTR)))
		return (-1);

	if (getmode(ino) == MODE_R(F_DIR) && esuper())
		goto nogood;

	if (ino->c_node.i_nlink == 0xFFFF) {
		udata.u_error = EMLINK;
		goto nogood;
	}

	/* Make sure file2 doesn't exist, and get its parent */
	if ((ino2 = n_open(name2, &parent2)) != NULL) {
		i_deref(ino2);
		i_deref(parent2);
		udata.u_error = EEXIST;
		goto nogood;
	}

	if (!parent2)
		goto nogood;

	if (ino->c_dev != parent2->c_dev) {
		i_deref(parent2);
		udata.u_error = EXDEV;
		goto nogood;
	}

	filename(name2, fname);

	if (!ch_link(parent2, "", fname, ino)) {
		i_deref(parent2);
		goto nogood;
	}

	/* Update the link count. */
	++ino->c_node.i_nlink;
	wr_inode(ino);
	setftime(ino, C_TIME);

	i_deref(parent2);
	i_deref(ino);
	return 0;

      nogood:
	i_deref(ino);
	return -1;
}

#undef name1
#undef name2


/*******************************************
  fcntl (fd, request, data)        Function 47
  int  fd;
  int  request;
  char *data;
 ********************************************/
#define fd (int)udata.u_argn
#define request (int)udata.u_argn1
#define data (int)udata.u_argn2

arg_t _fcntl(void)
{
	uint8_t *acc;
	int newd;

	if (getinode(fd) == NULLINODE)
		return (-1);

	acc = &of_tab[udata.u_files[fd]].o_access;
	switch (request) {
	case F_SETFL:
		*acc =
		    (*acc & ~(O_APPEND | O_NDELAY)) | (data &
						       (O_APPEND |
							O_NDELAY));
		return 0;
	case F_GETFL:
		return data;
	case F_GETFD:
		return udata.u_cloexec & (1 << fd) ? O_CLOEXEC : 0;
	case F_SETFD:
		if (data & O_CLOEXEC)
			udata.u_cloexec |= (1 << fd);
		else
			udata.u_cloexec &= (1 << fd);
		return 0;
	case F_DUPFD:
		if ((newd = uf_alloc_n(data)) == -1)
			return (-1);
		udata.u_files[newd] = udata.u_files[fd];
		++of_tab[udata.u_files[fd]].o_refs;
		return 0;
	default:
		udata.u_error = EINVAL;
		return -1;
	}
}

#undef fd
#undef request
#undef data

/*******************************************
uname (buf, len)                 Function 54
char *buf;
uint16_t len;

We pass a set of \0 terminated strings, don't bother
with node name. Rest is up to the libc.
********************************************/

#define buf (char *)udata.u_argn
#define len (uint16_t)udata.u_argn1

arg_t _uname(void)
{
        uint16_t size = sizeof(sysinfo) + uname_len;
        if (size > len)
                size = len;
	sysinfo.memk = procmem;
	sysinfo.usedk = pagemap_mem_used();
	sysinfo.nproc = PTABSIZE;
	sysinfo.ticks = ticks_per_dsecond * 10;
	sysinfo.loadavg[0] = loadavg[0].average;
	sysinfo.loadavg[1] = loadavg[1].average;
	sysinfo.loadavg[2] = loadavg[2].average;
	uput(&sysinfo, buf, size);
	return size;
}

#undef buf

/**************************************
flock(fd, lockop)	    Function 60
int file;
int lockop;

Perform locking upon a file.
**************************************/

#define file (uint16_t)udata.u_argn
#define lockop (uint16_t)udata.u_argn1

arg_t _flock(void)
{
	inoptr ino;
	struct oft *o;
	staticfast uint8_t c;
	staticfast uint8_t lock;
	staticfast int self;

	lock = lockop & ~LOCK_NB;
	self = 0;

	if (lock > LOCK_UN) {
		udata.u_error = EINVAL;
		return -1;
	}

	if ((ino = getinode(file)) == NULLINODE)
		return -1;
	o = &of_tab[udata.u_files[file]];

	c = ino->c_flags & CFLOCK;

	/* Upgrades and downgrades. Check if we are in fact doing a no-op */
	if (o->o_access & O_FLOCK) {
		self = 1;
		/* Shared or exclusive to shared can't block and is easy */
		if (lock == LOCK_SH) {
			if (c == CFLEX)
				c = 1;
			goto done;
		}
		/* Exclusive to exclusive - no op */
		if (c == CFLEX && lock == LOCK_EX)
			return 0;
		/* Shared to exclusive - handle via the loop */
	}
		
		
	/* Unlock - drop the locks, mark us not a lock holder. Doesn't block */
	if (lockop == LOCK_UN) {
		o->o_access &= ~O_FLOCK;
		deflock(o);
		return 0;
	}

	do {
		/* Exclusive lock must have no holders */
		if (c == self && lock == LOCK_EX) {
			c = CFLEX;
			goto done;
		}
		if (c < CFMAX) {
			c++;
			goto done;
		}
		if (c == CFMAX) {
			udata.u_error = ENOLCK;
			return -1;
		}
		/* LOCK_NB is defined as O_NDELAY... */
		if (psleep_flags(&ino->c_flags, (lockop & LOCK_NB)))
			return -1;
		/* locks will hopefully have changed .. */
		c = ino->c_flags & CFLOCK;
	} while (1);

done:
	if (o->o_access & O_FLOCK)
		deflock(o);
	ino->c_flags &= ~CFLOCK;
	ino->c_flags |= c;
	o->o_access |= O_FLOCK;
	wakeup(&ino->c_flags);
	return 0;
}


#undef file
#undef lockop
