#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

/*
 *	File system related calls that are not continually used (so
 *	we can potentially bank them out if we do a 32K/32K mode)
 */

static int16_t chdiroot_op(inoptr ino, inoptr * p)
{
	if (getmode(ino) != F_DIR) {
		udata.u_error = ENOTDIR;
		i_deref(ino);
		return (-1);
	}
	i_deref(*p);
	*p = ino;
	return 0;
}

/*******************************************
  chdir (dir)                      Function 10
  char *dir;
 ********************************************/
#define dir (char *)udata.u_argn

int16_t _chdir(void)
{
	inoptr newcwd;

	if (!(newcwd = n_open(dir, NULLINOPTR)))
		return (-1);
	return chdiroot_op(newcwd, &udata.u_cwd);
}

#undef dir

/*******************************************
  fchdir(fd)                      Function 48
  int fd;
 ********************************************/
#define fd (int16_t)udata.u_argn

int16_t _fchdir(void)
{
	inoptr newcwd;

	if ((newcwd = getinode(fd)) == NULLINODE)
		return (-1);
	i_ref(newcwd);
	return chdiroot_op(newcwd, &udata.u_cwd);
}

#undef fd

/*******************************************
  chroot (dir)                      Function 46
  char *dir;
 ********************************************/
#define dir (char *)udata.u_argn

int16_t _chroot(void)
{
	inoptr newroot;

	if (esuper())
		return (-1);
	if (!(newroot = n_open(dir, NULLINOPTR)))
		return (-1);
	return chdiroot_op(newroot, &udata.u_root);
}

#undef dir


/*******************************************
  mknod (name, mode, dev)           Function 4
  char  *name;
  int16_t mode;
  int16_t dev;
 ********************************************/
#define name (char *)udata.u_argn
#define mode (int16_t)udata.u_argn1
#define dev  (int16_t)udata.u_argn2

int16_t _mknod(void)
{
	inoptr ino;
	inoptr parent;
	char fname[FILENAME_LEN + 1];

	udata.u_error = 0;

	if (!super() && ((mode & F_MASK) != F_PIPE)) {
		udata.u_error = EPERM;
		goto nogood3;
	}
	if ((ino = n_open(name, &parent)) != NULL) {
		udata.u_error = EEXIST;
		goto nogood;
	}

	if (!parent) {
		udata.u_error = ENOENT;
		return (-1);
	}

	filename(name, fname);
	if ((ino = newfile(parent, fname)) != NULL)
		goto nogood3;	/* parent inode is derefed in newfile. SN */

	/* Initialize mode and dev */
	ino->c_node.i_mode = mode & ~udata.u_mask;
	ino->c_node.i_addr[0] = isdevice(ino) ? dev : 0;
	setftime(ino, A_TIME | M_TIME | C_TIME);
	wr_inode(ino);

	i_deref(ino);
	return (0);

      nogood:
	i_deref(ino);
	i_deref(parent);
      nogood3:
	return (-1);
}

#undef name
#undef mode
#undef dev

/*******************************************
  access (path, mode)              Function 12        ?
  char  *path;
  int16_t mode;
 ********************************************/
#define path (char *)udata.u_argn
#define mode (int16_t)udata.u_argn1

int16_t _access(void)
{
	inoptr ino;
	uint16_t euid;
	uint16_t egid;
	uint16_t retval;

	if ((mode & 07) && !ugetc(path)) {
		udata.u_error = ENOENT;
		return (-1);
	}

	/* Temporarily make eff. id real id. */
	euid = udata.u_euid;
	egid = udata.u_egid;
	udata.u_euid = udata.u_ptab->p_uid;
	udata.u_egid = udata.u_gid;

	if (!(ino = n_open(path, NULLINOPTR))) {
		retval = -1;
		goto nogood;
	}

	retval = 0;
	if (~getperm(ino) & (mode & 07)) {
		udata.u_error = EPERM;
		retval = -1;
	}

	i_deref(ino);
      nogood:
	udata.u_euid = euid;
	udata.u_egid = egid;

	return (retval);
}

#undef path
#undef mode

#define mode (int16_t)udata.u_argn1

static int16_t chmod_op(inoptr ino)
{
	if (ino->c_node.i_uid != udata.u_euid && esuper())
		return (-1);

	ino->c_node.i_mode =
	    (mode & MODE_MASK) | (ino->c_node.i_mode & F_MASK);
	setftime(ino, C_TIME);
	return 0;
}

#undef mode

/*******************************************
  chmod (path, mode)               Function 13
  char  *path;
  int16_t mode;
 ********************************************/
#define path (char *)udata.u_argn

int16_t _chmod(void)
{
	inoptr ino;
	int ret;

	if (!(ino = n_open(path, NULLINOPTR)))
		return (-1);
	ret = chmod_op(ino);
	i_deref(ino);
	return ret;
}

#undef path

/*******************************************
  fchmod (path, mode)               Function 49
  int fd;
  int16_t mode;
 ********************************************/
#define fd (int16_t)udata.u_argn

int16_t _fchmod(void)
{
	inoptr ino;

	if ((ino = getinode(fd)) == NULLINODE)
		return (-1);

	return chmod_op(ino);
}

#undef fd

#define owner (int16_t)udata.u_argn1
#define group (int16_t)udata.u_argn2

static int chown_op(inoptr ino)
{
	if (ino->c_node.i_uid != udata.u_euid && esuper())
		return (-1);
	ino->c_node.i_uid = owner;
	ino->c_node.i_gid = group;
	setftime(ino, C_TIME);
	return 0;
}

#undef owner
#undef group

/*******************************************
  chown (path, owner, group)       Function 14        ?
  char *path;
  int  owner;
  int  group;
 ********************************************/
#define path (char *)udata.u_argn

int16_t _chown(void)
{
	inoptr ino;
	int ret;

	if (!(ino = n_open(path, NULLINOPTR)))
		return (-1);
	ret = chown_op(ino);
	i_deref(ino);
	return ret;
}

#undef path

/*******************************************
  fchown (path, owner, group)       Function 50
  int fd;
  int  owner;
  int  group;
 ********************************************/
#define fd (int16_t)udata.u_argn

int16_t _fchown(void)
{
	inoptr ino;

	if ((ino = getinode(fd)) == NULLINODE)
		return (-1);
	return chown_op(ino);
}

#undef fd


/*******************************************
  utime (file, buf)                Function 43
  char *file;
  char *buf;
 ********************************************/
#define file (char *)udata.u_argn
#define buf (char *)udata.u_argn1

int16_t _utime(void)
{
	inoptr ino;
	time_t t[2];

	if (!(ino = n_open(file, NULLINOPTR)))
		return (-1);
	/* Special case in the Unix API - NULL means now */
	if (buf) {
	        if (ino->c_node.i_uid != udata.u_euid && esuper())
			goto out;
		if (!valaddr(buf, 2 * sizeof(time_t)))
			goto out2;
		uget(buf, t, 2 * sizeof(time_t));
	} else {
	        if (!(getperm(ino) & OTH_WR))
			goto out;
	        rdtime(&t[0]);
	        memcpy(&t[1], &t[0], sizeof(t[1]));
        }
	/* FIXME: needs updating once we pack top bits
	   elsewhere in the inode */
	ino->c_node.i_atime = t[0].low;
	ino->c_node.i_mtime = t[1].low;
	setftime(ino, C_TIME);
	i_deref(ino);
	return (0);
out:
	udata.u_error = EPERM;
out2:
	i_deref(ino);
	return -1;
}

#undef file
#undef buf

/*******************************************
  acct(fd)	                 Function 61
  int16_t fd;
  
  Process accounting. Differs from SYS5 in
  that we pass an fd and hide the opening
  in the C library code.
 *******************************************/
#define fd (int16_t)udata.u_argn

int16_t _acct(void)
{
#ifdef CONFIG_ACCT
        inoptr inode;
        if (esuper())
                return -1;
        if (acct_fh != -1)
                oft_deref(acct_fh);
        if (fd != -1) {
                if ((inode = getinode(fd)) == NULLINODE)
                        return -1;
                if (getmode(inode) != F_REG) {
                        udata.u_error = EINVAL;
                        return -1;
                }
        	acct_fh = udata.u_files[fd];
        	++of_tab[acct_fh].o_refs;
        }
	return 0;
#else
        udata.u_error = EINVAL;
        return -1;
#endif        
}

#undef fd

/* Special system call returns super-block of given filesystem
 * for users to determine free space, etc.  Should be replaced
 * with a sync() followed by a read of block 1 of the device.
 */
/*******************************************
  getfsys (dev, buf)               Function 22
  int16_t  dev;
  struct filesys *buf;
 ********************************************/
#define dev (uint16_t)udata.u_argn
#define buf (struct filesys *)udata.u_argn1

int16_t _getfsys(void)
{
        struct mount *m = fs_tab_get(dev);
	if (m == NULL || m->m_dev == NO_DEVICE) {
		udata.u_error = ENXIO;
		return (-1);
	}
	uput((char *) m->m_fs, (char *) buf, sizeof(struct filesys));
	return (0);
}

#undef dev
#undef buf

/*******************************************
open (name, flag, mode)           Function 1
char *name;
int16_t flag;
int16_t mode;
********************************************/
#define name (char *)udata.u_argn
#define flag (uint16_t)udata.u_argn1
#define mode (uint16_t)udata.u_argn2

int16_t _open(void)
{
	int8_t uindex;
	int8_t oftindex;
	staticfast inoptr ino;
	inoptr itmp;
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
		/* New file */
		if (!(flag & O_CREAT)) {
			udata.u_error = ENOENT;
			goto cantopen;
		}
		filename(name, fname);
		/* newfile drops parent for us */
		if (parent && (ino = newfile(parent, fname))) {
			ino->c_node.i_mode =
			    (F_REG | (mode & MODE_MASK & ~udata.u_mask));
			setftime(ino, A_TIME | M_TIME | C_TIME);
			wr_inode(ino);
		} else {
			udata.u_error = ENFILE;	/* FIXME, should be set in newfile
						   not bodged to a guessed code */
			goto cantopen;
		}
	}
	of_tab[oftindex].o_inode = ino;

	perm = getperm(ino);
	if ((r && !(perm & OTH_RD)) || (w && !(perm & OTH_WR))) {
		udata.u_error = EPERM;
		goto cantopen;
	}
	if (getmode(ino) == F_DIR && w) {
		udata.u_error = EISDIR;
		goto cantopen;
	}
	itmp = ino;
	/* d_open may block and thus ino may become invalid as may
	   parent (but we don't need it again) */
	if (isdevice(ino)
	    && d_open((int) ino->c_node.i_addr[0], flag) != 0) {
		udata.u_error = ENXIO;
		goto cantopen;
	}
	/* get the static pointer back */
	ino = itmp;
	if (trunc && getmode(ino) == F_REG) {
		f_trunc(ino);
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
	if (getmode(ino) == F_PIPE && of_tab[oftindex].o_refs == 1
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

int16_t _link(void)
{
	inoptr ino;
	inoptr ino2;
	inoptr parent2;
	char fname[FILENAME_LEN + 1];

	if (!(ino = n_open(name1, NULLINOPTR)))
		return (-1);

	if (getmode(ino) == F_DIR && esuper())
		goto nogood;

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

int16_t _fcntl(void)
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

int16_t _uname(void)
{
        uint16_t size = sizeof(sysinfo) + uname_len;
        if (size > len)
                size = len;
	sysinfo.memk = procmem;
	sysinfo.usedk = pagemap_mem_used();
	sysinfo.nproc = PTABSIZE;
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

int16_t _flock(void)
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