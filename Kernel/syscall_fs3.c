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
#define name (uint8_t *)udata.u_argn
#define flag (uint16_t)udata.u_argn1
#define mode (uint16_t)udata.u_argn2

arg_t _open(void)
{
	int_fast8_t uindex;
	int_fast8_t oftindex;
	inoptr ino;
	int16_t perm;
	staticfast inoptr parent;
	int r;
	int w;
	int j;

	parent = NULLINODE;

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

	ino = n_open_lock(name, &parent);
	if (ino) {
		/* We hold a reference to the found inode. but we don't need
		   one to the parent as we have nothing to create */
		i_deref(parent);
		/* O_EXCL test */
		if ((flag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
			udata.u_error = EEXIST;
			i_deref(ino);
			goto idrop;
		}
	} else {
		/* We hold a reference to the parent possibly */
		/* The n_open failed */
		if (udata.u_error == EFAULT || parent == NULL)
			goto pdrop;

		/* New file */
		if (!(flag & O_CREAT)) {
			udata.u_error = ENOENT;
			goto pdrop;
		}
		/* newfile drops parent for us, ino is now locked */
		ino = newfile(parent, lastname);
		if (!ino) {
			/* on error, newfile sets udata.u_error */
			goto ideref;
		}

		/* new inode, successfully created */
		ino->c_node.i_mode =
		    (F_REG | (mode & MODE_MASK & ~udata.u_mask));
		setftime(ino, A_TIME | M_TIME | C_TIME);
		wr_inode(ino);
		perm = getperm(ino);

		/* In the Unix world a creat() of a file with no permissions
		   still produces a file which you can use so we must
		   do this test entirely on the existig file side */
		if ((r && !(perm & OTH_RD)) || (w && !(perm & OTH_WR))) {
			udata.u_error = EACCES;
			goto ideref;
		}
	}
	/* However we arrived here we hold a reference to the new inode
	   and we have dropped the parent reference. The object exists
	   and we will put it into the of table. Once there the oft clean
	   up will dereference it, so from this point onwards error handling
	   and reference management is simple */

	/* Book our slot in case we block opening a device */
	of_tab[oftindex].o_inode = ino;

	if (w) {
		if (getmode(ino) == MODE_R(F_DIR)) {
			udata.u_error = EISDIR;
			goto idrop;
		}
		/* Special case - devices on a read only file system may
		   be opened read/write */
		if (!isdevice(ino) && (ino->c_flags & CRDONLY)) {
			udata.u_error = EROFS;
			goto idrop;
		}
	}
	/* Opening a device node is special. We don't really open the thing
	 * on the disk, instead it's a pointer to some other object that is
	 * not really in the file system space.
	 */
	if (isdevice(ino)) {
		inoptr *iptr = &of_tab[oftindex].o_inode;
		/* d_open may block and thus ino may become invalid as may
		   parent (but we don't need it again). It may also be changed
		   by the call to dev_openi. /dev/tty in particular does this
		   to assign device instances */
		i_unlock(*iptr);
		if (dev_openi(iptr, flag) != 0)
			goto cantopen;
		/* May have changed */
		/* get the static pointer back in case it changed via dev 
		   usage or just because we blocked */
		ino = *iptr;
		i_lock(ino);
	} else if (w && (flag & O_TRUNC) && getmode(ino) == MODE_R(F_REG)) {
		/* O_TRUNC applied to a writeable ordinary file causes the
		   file to be truncated back to zero size */
		if (f_trunc(ino))
			goto idrop;
		for (j = 0; j < OFTSIZE; ++j)
			/* Arguably should fix at read/write */
			if (of_tab[j].o_inode == ino)
				of_tab[j].o_ptr = 0;
	}
	/* Link our file descriptor to the of table slot */
	udata.u_files[uindex] = oftindex;
	/* Set up the other fields in the of table */
	of_tab[oftindex].o_ptr = 0;
	of_tab[oftindex].o_access = flag;	/* Save the low bits only */

	/* O_CLOEXEC updates the mask of file handles to close on execve()
	   so that you can handle this atomically */
	if (flag & O_CLOEXEC)
		udata.u_cloexec |= (1 << uindex);

	/* Update the number of readers and writers as we need this for
	   pipe behaviour */
	if (O_ACCMODE(flag) != O_RDONLY)
		ino->c_writers++;
	if (O_ACCMODE(flag) != O_WRONLY)
		ino->c_readers++;

	i_unlock(ino);

	/* FIXME: ATIME ? */

	if (getmode(ino) == MODE_R(F_PIPE)) {
		/* There are special open semantics on named pipes. A read
		   open of a named pipe that has no writer blocks */
		if (of_tab[oftindex].o_refs == 1
			    && !(flag & O_NDELAY)) {
			psleep(ino);
			if (chksigs()) {
				udata.u_error = EINTR;
				goto idrop;
			}
		}
		/* A read open of a named pipe with NDELAY that has no
		   writers fails with EXNIO */
		if ((ino->c_writers == 0) && (flag & O_NDELAY)) {
			udata.u_error = ENXIO;
			goto idrop;
		}
	}
	return (uindex);

pdrop:
	if (parent)
		i_deref(parent);
ideref:
	if (ino)
		i_deref(ino);
idrop:
	i_unlock(ino);
	/* Falls through and drops the reference count */
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
#define name1 (uint8_t *)udata.u_argn
#define name2 (uint8_t *)udata.u_argn1

arg_t _link(void)
{
	register inoptr ino;
	inoptr ino2;
	inoptr parent2;

	if (!(ino = n_open(name1, NULLINOPTR)))
		return (-1);

	if (getmode(ino) == MODE_R(F_DIR)) {
		udata.u_error = EISDIR;
		goto nogood;
	}

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

	i_lock(parent2);
	if (!ch_link(parent2, (uint8_t *)"", lastname, ino)) {
		i_unlock_deref(parent2);
		goto nogoodl;
	}

	/* Update the link count. */
	++ino->c_node.i_nlink;
	wr_inode(ino);
	setftime(ino, C_TIME);

	i_unlock_deref(parent2);
	i_deref(ino);
	return 0;

nogoodl:
	i_unlock(ino);
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
		return *acc;
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

#define buf (uint8_t *)udata.u_argn
#define len (uint16_t)udata.u_argn1

arg_t _uname(void)
{
        uint16_t size = sizeof(sysinfo);
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
	regptr struct oft *o;
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
