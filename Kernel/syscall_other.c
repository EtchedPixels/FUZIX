#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>

/*
 *	More obscure syscalls that it might be useful to pull out of the main
 *	flow in case we ever need to bank any of them. Avoid putting anything
 *	that may sleep in here. We want this to potentially be pageable syscall
 *	helper. Right now its about 3.5K of material
 */


/*******************************************
rename (src, dst)                Function 59
char *src;
char *dst;

Rename a file within a filesystem, can be
a directory. Don't allow an object to be
put inside itself!
********************************************/

#define src (char *)udata.u_argn
#define dst (char *)udata.u_argn1

int16_t _rename(void)
{
	staticfast inoptr srci, srcp, dsti, dstp;
	char fname[FILENAME_LEN + 1];
	int ret = -1;

	srci = n_open(src, &srcp);
	/* Source must exist */
	if (!srci) {
		if (srcp)
			i_deref(srcp);
		return -1;
	}
	/* n_open will wipe u_rename if it walks that inode
	   so it tells us whether we are trying to create a loop */
	udata.u_rename = srci;
	/* Destination must not exist, but parent must */
	filename(dst, fname);
	dsti = n_open(dst, &dstp);
	/* Destination not found, but neither is the directory to
	   put the new node in -> so fail */
	if (dstp == NULLINODE)
		goto nogood3;
	/* Can't rename between devices */
	if (srci->c_dev != dstp->c_dev) {
		udata.u_error = EXDEV;
		goto nogood;
	}
	/* Can't rename one dir into another */
	if (udata.u_rename == NULL) {
		/* dst is within src */
		udata.u_error = EINVAL;
		goto nogood;
	}
	/* Check we can write the src dir, we don't want to fail on
	   ch_link */
	if (!(getperm(srcp) & OTH_WR) && esuper()) {
		udata.u_error = EPERM;
		goto nogood;
	}
	/* Destination exists ? If so we must remove it if possible */
	if (dsti) {
		if (unlinki(dsti, dstp, fname) == -1)
			goto nogood;
		/* Drop the reference to the unlinked file */
		i_deref(dsti);
	}
	/* Ok we may proceed: we set up fname earlier */
	if (!ch_link(dstp, "", fname, srci))
		goto nogood2;
	filename(src, fname);
	/* A fail here is bad */
	if (!ch_link(srcp, fname, "", NULLINODE)) {
		kputs("WARNING: rename: unlink fail\n");
		goto nogood2;
	}
	/* get it onto disk - probably overkill */
	wr_inode(dstp);
	wr_inode(srcp);
	_sync();
	ret = 0;
      nogood2:
	i_deref(dstp);
      nogood3:
	i_deref(srci);
	i_deref(srcp);
	return ret;
      nogood:
	if (dsti)
		i_deref(dsti);
	goto nogood2;
}

#undef src
#undef dst

/*******************************************
  mkdir (path, mode)	        Function 51
  char *path;
  int16_t mode;
 ********************************************/

#define name (char *)udata.u_argn
#define mode (int16_t)udata.u_argn1

int16_t _mkdir(void)
{
	inoptr ino;
	inoptr parent;
	char fname[FILENAME_LEN + 1];

	if ((ino = n_open(name, &parent)) != NULL) {
		udata.u_error = EEXIST;
		goto nogood;
	}

	if (!parent) {
		udata.u_error = ENOENT;
		return (-1);
	}

	filename(name, fname);

	i_ref(parent);		/* We need it again in a minute */
	if (!(ino = newfile(parent, fname))) {
		i_deref(parent);
		goto nogood2;	/* parent inode is derefed in newfile. */
	}

	/* Initialize mode and dev */
	ino->c_node.i_mode = F_DIR | 0200;	/* so ch_link is allowed */
	setftime(ino, A_TIME | M_TIME | C_TIME);
	if (ch_link(ino, "", ".", ino) == 0 ||
	    ch_link(ino, "", "..", parent) == 0)
		goto cleanup;

	/* Link counts and permissions */
	ino->c_node.i_nlink = 2;
	parent->c_node.i_nlink++;
	ino->c_node.i_mode = ((mode & ~udata.u_mask) & MODE_MASK) | F_DIR;
	i_deref(parent);
	wr_inode(ino);
	i_deref(ino);
	return (0);

      cleanup:
	if (!ch_link(parent, fname, "", NULLINODE))
		kprintf("_mkdir: bad rec\n");
	/* i_deref will put the blocks */
	ino->c_node.i_nlink = 0;
	wr_inode(ino);
      nogood:
	i_deref(ino);
      nogood2:
	i_deref(parent);
	return (-1);

}

#undef name
#undef mode

/*******************************************
rmdir (path)                     Function 52
char *path;
********************************************/
#define path (char *)udata.u_argn

int16_t _rmdir(void)
{
	inoptr ino;
	inoptr parent;
	char fname[FILENAME_LEN + 1];

	ino = n_open(path, &parent);

	/* It and its parent must exist */
	if (!(parent && ino)) {
		if (parent)	/* parent exist */
			i_deref(parent);
		udata.u_error = ENOENT;
		return (-1);
	}

	/* Fixme: check for rmdir of /. - ditto for unlink ? */

	/* Not a directory */
	if (getmode(ino) != F_DIR) {
		udata.u_error = ENOTDIR;
		goto nogood;
	}

	/* Busy */
	if (ino->c_node.i_nlink != 2) {
		udata.u_error = ENOTEMPTY;
		goto nogood;
	}

	/* Parent must be writable */
	if (!getperm(parent) & OTH_WR)
		goto nogood;

	/* Remove the directory entry */
	filename(path, fname);
	if (!ch_link(parent, fname, "", NULLINODE))
		goto nogood;

	/* We are unused, parent is now one link down (removal of ..) */
	ino->c_node.i_nlink = 0;

	/* Decrease the link count of the parent inode */
	if (!(parent->c_node.i_nlink--)) {
		parent->c_node.i_nlink += 2;
		kprintf("_rmdir: bad nlink\n");
	}
	setftime(ino, C_TIME);
	wr_inode(parent);
	wr_inode(ino);
	i_deref(parent);
	i_deref(ino);
	return (0);

      nogood:
	i_deref(parent);
	i_deref(ino);
	return (-1);
}

#undef path


/*******************************************
  mount (spec, dir, flags)       Function 33
  char *spec;
  char *dir;
  int  flags;
 ********************************************/
#define spec (char *)udata.u_argn
#define dir (char *)udata.u_argn1
#define flags (int)udata.u_argn2

int16_t _mount(void)
{
	inoptr sino, dino;
	uint16_t dev;

	if (esuper()) {
		return (-1);
	}

	if (!(sino = n_open(spec, NULLINOPTR)))
		return (-1);

	if (!(dino = n_open(dir, NULLINOPTR))) {
		i_deref(sino);
		return (-1);
	}

	if (getmode(sino) != F_BDEV) {
		udata.u_error = ENOTBLK;
		goto nogood;
	}

	if (getmode(dino) != F_DIR) {
		udata.u_error = ENOTDIR;
		goto nogood;
	}

	dev = (int) sino->c_node.i_addr[0];

	if (!validdev(dev) || d_open(dev, (flags & MS_RDONLY) ? O_RDONLY : O_RDWR)) {
		udata.u_error = ENXIO;
		goto nogood;
	}

	if (fs_tab_get(dev) || dino->c_refs != 1 || dino->c_num == ROOTINODE) {
		udata.u_error = EBUSY;
		goto nogood;
	}

	_sync();

	if (fmount(dev, dino, flags)) {
		udata.u_error = EBUSY;
		goto nogood;
	}

	i_deref(dino);
	i_deref(sino);
	return (0);

      nogood:
	i_deref(dino);
	i_deref(sino);
	return (-1);
}

#undef spec
#undef dir
#undef flags


/*******************************************
  umount (spec)                    Function 34
  char *spec;
 ********************************************/
#define spec (char *)udata.u_argn

int16_t _umount(void)
{
	inoptr sino;
	uint16_t dev;
	inoptr ptr;
	struct mount *mnt;

	if (esuper())
		return (-1);

	if (!(sino = n_open(spec, NULLINOPTR)))
		return (-1);

	if (getmode(sino) != F_BDEV) {
		udata.u_error = ENOTBLK;
		goto nogood;
	}

	dev = (int) sino->c_node.i_addr[0];
	if (!validdev(dev)) {
		udata.u_error = ENXIO;
		goto nogood;
	}

	mnt = fs_tab_get(dev);
	if (mnt == NULL) {
		udata.u_error = EINVAL;
		goto nogood;
	}

	for (ptr = i_tab; ptr < i_tab + ITABSIZE; ++ptr)
		if (ptr->c_refs > 0 && ptr->c_dev == dev) {
			udata.u_error = EBUSY;
			goto nogood;
		}

	_sync();

	mnt->m_fs->s_mounted = 0;
	i_deref(mnt->m_fs->s_mntpt);
	/* Give back the buffer we pinned at mount time */
	bfree((bufptr)mnt->m_fs, 2);
	/* Vanish the entry */
	mnt->m_dev = NO_DEVICE;
	i_deref(sino);
	return 0;

      nogood:
	i_deref(sino);
	return -1;
}

#undef spec


/* User's execve() call. All other flavors are library routines. */
/*******************************************
execve (name, argv, envp)        Function 23
char *name;
char *argv[];
char *envp[];
********************************************/
#define name (char *)udata.u_argn
#define argv (char **)udata.u_argn1
#define envp (char **)udata.u_argn2

int16_t _execve(void)
{
	inoptr ino, emu_ino;
	unsigned char *buf;
	blkno_t blk;
	char **nargv;		/* In user space */
	char **nenvp;		/* In user space */
	struct s_argblk *abuf, *ebuf;
	int16_t (**sigp) ();
	int argc;
	uint16_t emu_size, emu_copy;
	uint8_t *progptr, *emu_ptr, *emu_base;
	int j;
	uint16_t top = (uint16_t)ramtop;
	uint8_t c;
	uint16_t blocks;

	kputs("execve\n");

	if (!(ino = n_open(name, NULLINOPTR)))
		return (-1);

	kputs("Found it\n");
	if (!((getperm(ino) & OTH_EX) &&
	      (ino->c_node.i_mode & F_REG) &&
	      (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)))) {
		udata.u_error = EACCES;
		goto nogood;
	}

	setftime(ino, A_TIME);

	/* Read in the first block of the new program */
	buf = bread(ino->c_dev, bmap(ino, 0, 1), 0);

    /****************************************
     * Get magic number into var magic
     * C3    : executable file no C/D sep.
     * 00FF  :     "        "  with C/D sep. (not supported in FUZIX)
     * other : maybe shell script (nogood2)
     ****************************************/
	if ((*buf & 0xff) != EMAGIC) {
		udata.u_error = ENOEXEC;
		goto nogood2;
	}
	kputs("Magic\n");

	/*
	 *	Executables might be CP/M or Fuzix (we don't support legacy
	 *	UZI binaries).
	 */
	if (buf[3] == 'F' && buf[4] == 'Z' && buf[5] == 'X' && buf[6] == '1') {
		top = buf[7] | ((unsigned int)buf[8] << 8);
		if (top == 0)	/* Legacy 'all space' binary */
			top = (uint16_t)ramtop;
		emu_ino = 0;	// no emulation, thanks
	} else {
#ifdef CONFIG_CPM_EMU
		// open the emulator code on disk
		emu_ino = kn_open(CPM_EMULATOR_FILENAME, NULLINOPTR);
		if (!emu_ino) {
			kprintf("Cannot load emulator: %s\n",
				CPM_EMULATOR_FILENAME);
			udata.u_error = ENOEXEC;
			goto nogood2;
		}
		top = (uint16_t)ramtop;
#else
		emu_size;
		emu_copy;
		emu_ptr;
		udata.u_error = ENOEXEC;
		goto nogood2;
#endif
	}

	/* Binary doesn't fit */
	if (top < ino->c_node.i_size + 1024) {
		udata.u_error = ENOMEM;
		goto nogood2;
	}

	/* Gather the arguments, and put them in temporary buffers. */
	abuf = (struct s_argblk *) tmpbuf();
	/* Put environment in another buffer. */
	ebuf = (struct s_argblk *) tmpbuf();

	/* Read args and environment from process memory */
	if (rargs(argv, abuf) || rargs(envp, ebuf))
		goto nogood3;	/* SN */

	/* This must be the last test as it makes changes if it works */
	if (pagemap_realloc(top))
		goto nogood3;

	/* From this point on we are commmited to the exec() completing */
	udata.u_top = top;

	/* setuid, setgid if executable requires it */
	if (ino->c_node.i_mode & SET_UID)
		udata.u_euid = ino->c_node.i_uid;

	if (ino->c_node.i_mode & SET_GID)
		udata.u_egid = ino->c_node.i_gid;

	/* We are definitely going to succeed with the exec,
	 * so we can start writing over the old program
	 */
	uput(buf, PROGBASE, 512);	/* Move 1st Block to user bank */
	brelse(buf);

	c = ugetc(PROGBASE);
	if (c != 0xC3)
		kprintf("Botched uput\n");

	/* At this point, we are committed to reading in and
	 * executing the program. */

	for (j = 0; j < UFTSIZE; ++j) {
		if (udata.u_cloexec & (1 << j))
			doclose(j);
	}
	udata.u_cloexec = 0;

#ifdef CONFIG_CPM_EMU
	// Load the CP/M emulator if it is required
	if (emu_ino) {
		emu_size = emu_ino->c_node.i_size;
		// round up to nearest multiple of 256 bytes, fit it in below ramtop
		emu_ptr =
		    (char *) (udata.u_top - ((emu_size + 255) & 0xff00));
		emu_base = emu_ptr;
		blk = 0;

		while (emu_size) {
			buf = bread(emu_ino->c_dev, bmap(emu_ino, blk, 1), 0);	// read block
			emu_copy = min(512, emu_size);
			uput(buf, emu_ptr, emu_copy);	// copy to userspace
			bufdiscard((bufptr) buf);
			brelse((bufptr) buf);	// release block
			// adjust pointers
			emu_ptr += emu_copy;
			emu_size -= emu_copy;
			blk++;
		}
		// close emulator file
		i_deref(emu_ino);

		/*
		 * zero out the remainder of memory between the top of the emulator and top
		 * of process memory
		 */

		uzero(emu_ptr, top - emu_ptr);
	} else
#endif
	{
		emu_base = (uint8_t *)top;
	}

	/* emu_base now points at the byte after the last byte the program can occupy */

	/*
	 *  Read in the rest of the program, block by block
	 *  We use bufdiscard so that we load the entire app through the
	 *  same buffer to avoid cycling our small cache on this. Indirect blocks
	 *  will still be cached. - Hat tip to Steve Hosgood's OMU for that trick
	 */
	progptr = PROGBASE + 512;	// we copied the first block already

	/* Compute this once otherwise each loop we must recalculate this
	   as the compiler isn't entitled to assume the loop didn't change it */
	blocks = ino->c_node.i_size >> 9;

	for (blk = 1; blk <= blocks; ++blk) {
		buf = bread(ino->c_dev, bmap(ino, blk, 1), 0);
		uput(buf, progptr, 512);
		bufdiscard((bufptr) buf);
		brelse((bufptr) buf);
		progptr += 512;
	}
	i_deref(ino);
	udata.u_break = (int) progptr;	//  Set initial break for program

	// zero all remaining process memory above the last block loaded.
	uzero(progptr, emu_base - progptr);

	// Turn off caught signals
	for (sigp = udata.u_sigvec; sigp < udata.u_sigvec + NSIGS; ++sigp)
		*sigp = SIG_DFL;

	// place the arguments, environment and stack at the top of userspace memory,

	// Write back the arguments and the environment
	nargv = wargs(((char *) emu_base - 2), abuf, &argc);
	nenvp = wargs((char *) (nargv), ebuf, NULL);

	// Fill in udata.u_name with Program invocation name
	uget((void *) ugetw(nargv), udata.u_name, 8);
	memcpy(udata.u_ptab->p_name, udata.u_name, 8);

	brelse(abuf);
	brelse(ebuf);

	// Shove argc and the address of argv just below envp
	uputw((uint16_t) nargv, nenvp - 1);
	uputw((uint16_t) argc, nenvp - 2);

	// Set stack pointer for the program
	udata.u_isp = nenvp - 2;

	// Start execution (never returns)
#ifdef CONFIG_CPM_EMU
	if (emu_ino)
		doexec(emu_base);
	else
#endif
		doexec(PROGBASE);

	// tidy up in various failure modes:
      nogood3:
	brelse(abuf);
	brelse(ebuf);
#ifdef CONFIG_CPM_EMU
	if (emu_ino)
		i_deref(emu_ino);
#endif
      nogood2:
	brelse(buf);
      nogood:
	i_deref(ino);
	return (-1);
}

#undef name
#undef argv
#undef envp

/* SN    TODO      max (1024) 512 bytes for argv
               and max  512 bytes for environ
*/

bool rargs(char **userspace_argv, struct s_argblk * argbuf)
{
	char *ptr;		/* Address of base of arg strings in user space */
	uint8_t c;
	uint8_t *bufp;

	argbuf->a_argc = 0;	/* Store argc in argbuf */
	bufp = argbuf->a_buf;

	while ((ptr = (char *) ugetw(userspace_argv++)) != NULL) {
		++(argbuf->a_argc);	/* Store argc in argbuf. */
		do {
			*bufp++ = c = ugetc(ptr++);
			if (bufp > argbuf->a_buf + 500) {
				udata.u_error = E2BIG;
				return true;	// failed
			}
		}
		while (c);
	}
	argbuf->a_arglen = bufp - (uint8_t *)argbuf->a_buf;	/*Store total string size. */
	return false;		// success
}


char **wargs(char *ptr, struct s_argblk *argbuf, int *cnt)	// ptr is in userspace
{
	char **argv;		/* Address of users argv[], just below ptr */
	int argc, arglen;
	char **argbase;
	uint8_t *sptr;

	sptr = argbuf->a_buf;

	/* Move them into the users address space, at the very top */
	ptr -= (arglen = argbuf->a_arglen);

	if (arglen) {
		uput(sptr, ptr, arglen);
	}

	/* Set argv to point below the argument strings */
	argc = argbuf->a_argc;
	argbase = argv = (char **) ptr - (argc + 1);

	if (cnt) {
		*cnt = argc;
	}

	/* Set each element of argv[] to point to its argument string */
	while (argc--) {
		uputw((uint16_t) ptr, argv++);
		if (argc) {
			do
				++ptr;
			while (*sptr++);
		}
	}
	uputw(0, argv);		/*;;26Feb- Add Null Pointer to end of array */
	return ((char **) argbase);
}

/*******************************************
profil (samples, size, offset, scale) Function 56
char *samples;
uint16_t offset;
uint16_t size;
uint8_t scale;
********************************************/

#define samples (char *)udata.u_argn
#define offset 	(usize_t)udata.u_argn1
#define size 	(usize_t)udata.u_argn2
#define scale	(uint16_t)udata.u_argn3

int16_t _profil(void)
{
#ifdef CONFIG_PROFIL
	/* For performance reasons scale as
	   passed to the kernel is a shift value
	   not a divider */
	ptptr p = udata.u_ptab;

	if (scale == 0) {
		p->p_profscale = scale;
		return 0;
	}

	if (!valaddr(samples, size >> (scale - 1)))
		return -1;

	p->p_profscale = scale;
	p->p_profbuf = samples;
	p->p_profsize = size;
	p->p_profoff = offset;
	return 0;
#else
	return -1;
#endif
}

#undef samples
#undef offset
#undef size
#undef scale

/*******************************************
uadmin(cmd, func, ptr)           Function 57
int16_t cmd;
int16_t func;
char *ptr;
********************************************/
#define cmd (int16_t)udata.u_argn
#define func (int16_t)udata.u_argn1
#define ptr  (char *)udata.u_argn2

int16_t _uadmin(void)
{
	if (!esuper())
		return -1;
	/* Wants moving into machine specific files */
	if (cmd == A_SHUTDOWN || cmd == A_REBOOT || cmd == A_DUMP) {
		_sync();
		trap_monitor();
	}
	/* We don't do SWAPCTL yet */
	udata.u_error = EINVAL;
	return -1;
}


/*******************************************
nice (pri)                     Function 58
int16_t pri;
********************************************/
#define pri (int16_t)udata.u_argn

int16_t _nice(void)
{
	ptptr p = udata.u_ptab;
	int16_t np;

	if (pri < 0 && !esuper())
		return -1;
	np = p->p_nice + pri;
	if (np < -20)
		np = -20;
	if (np > 19)
		np = 19;

	p->p_nice = np;
	p->p_priority = MAXTICKS - (np >> 2);
	return 20 + np;		/* avoid errno confusion */
}

#undef pri
