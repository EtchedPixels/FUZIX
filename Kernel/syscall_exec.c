#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

static int bload(inoptr i, uint16_t bl, uint16_t base, uint16_t len)
{
	blkno_t blk;
	while(len) {
		uint16_t cp = min(len, 512);
		blk = bmap(i, bl, 1);
		if (blk == NULLBLK)
			uzero((uint8_t *)base, 512);
		else {
#ifdef CONFIG_LEGACY_EXEC
			uint8_t *buf;
			buf = bread(i->c_dev, blk, 0);
			if (buf == NULL) {
				kputs("bload failed.\n");
				return -1;
			}
			uput(buf, (uint8_t *)base, cp);
			bufdiscard((bufptr)buf);
			brelse((bufptr)buf);
#else
			/* Might be worth spotting sequential blocks and
			   merging ? */
			udata.u_offset = (off_t)blk << 9;
			udata.u_count = 512;
			udata.u_base = (uint8_t *)base;
			if (cdread(i->c_dev, 0) < 0) {
				kputs("bload failed.\n");
				return -1;
			}
#endif
		}
		base += cp;
		len -= cp;
		bl++;
	}
	return 0;
}

static void close_on_exec(void)
{
	int j;
	for (j = UFTSIZE - 1; j >= 0; --j) {
		if (udata.u_cloexec & (1 << j))
			doclose(j);
	}
	udata.u_cloexec = 0;
}

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
	staticfast inoptr ino;
	staticfast unsigned char *buf;
	char **nargv;		/* In user space */
	char **nenvp;		/* In user space */
	struct s_argblk *abuf, *ebuf;
	int argc;
	uint16_t progptr;
	staticfast uint16_t top;
	uint16_t bin_size;	/* Will need to be bigger on some cpus */
	uint16_t bss;
	uint8_t *p;

	top = ramtop;

	if (!(ino = n_open(name, NULLINOPTR)))
		return (-1);

	if (!((getperm(ino) & OTH_EX) &&
	      (ino->c_node.i_mode & F_REG) &&
	      (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)))) {
		udata.u_error = EACCES;
		goto nogood;
	}

	setftime(ino, A_TIME);

	if (ino->c_node.i_size == 0) {
		udata.u_error = ENOEXEC;
		goto nogood;
	}

	/* Read in the first block of the new program */
	buf = bread(ino->c_dev, bmap(ino, 0, 1), 0);

	/* Magic numbers
		0xC3 xx xx	- Z80 with 0x100 entry
		0x4C xx xx	- 6502
		0x0E xx xx	- 6809

	   followed by a base page for the executable

	*/
	if (*buf  != EMAGIC && *buf != EMAGIC_2) {
		udata.u_error = ENOEXEC;
		goto nogood2;
	}

	/*
	 *	Executables must be in FUZIX format (we'll let the CP/M emul
	 *	wrap the binaries and do the emulator load so we can clean up
	 *	all the kernel code for this case). We don't really want to end
	 *	up with CP/M, o65 and other emulations in kernel!
	 *
	 *	Use p to persuade sdcc not to generate shite code
	 */
	p = buf + 3;

	if (*p++ != 'F' || *p++ != 'Z' || *p++ != 'X' || *p++ !='1' || 
		/* Don't load binaries for the wrong base page, eg spectrum
		   binaries on a sane box. 0 indicates a relocatable binary */
		(*p && *p != PROGLOAD >> 8)) {
		udata.u_error = ENOEXEC;
		goto nogood2;
	}
	top = *++p + (*p << 8);
	if (top == 0)	/* Legacy 'all space' binary */
		top = ramtop;
	else	/* Requested an amount, so adjust for the base */
		top += PROGLOAD;

	bss = buf[14] | (buf[15] << 8);

	/* Binary doesn't fit */
	bin_size = ino->c_node.i_size;
	progptr = bin_size + 1024 + bss;
	if (top - PROGBASE < progptr || progptr < bin_size) {
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
	if (pagemap_realloc(top - MAPBASE))
		goto nogood3;

	/* From this point on we are commmited to the exec() completing */
	udata.u_top = top;

	/* setuid, setgid if executable requires it */
	if (ino->c_node.i_mode & SET_UID)
		udata.u_euid = ino->c_node.i_uid;

	if (ino->c_node.i_mode & SET_GID)
		udata.u_egid = ino->c_node.i_gid;

	/* FIXME: In the execve case we may on some platforms have space
	   below PROGLOAD to clear... */

	/* We are definitely going to succeed with the exec,
	 * so we can start writing over the old program
	 */
	uput(buf, (uint8_t *)PROGLOAD, 512);	/* Move 1st Block to user bank */
	brelse(buf);

	/* At this point, we are committed to reading in and
	 * executing the program. */

	close_on_exec();

	/*
	 *  Read in the rest of the program, block by block
	 *  We use bufdiscard so that we load the entire app through the
	 *  same buffer to avoid cycling our small cache on this. Indirect blocks
	 *  will still be cached. - Hat tip to Steve Hosgood's OMU for that trick
	 */
	progptr = PROGLOAD + 512;	// we copied the first block already

	/* Compute this once otherwise each loop we must recalculate this
	   as the compiler isn't entitled to assume the loop didn't change it */

	if (bin_size > 512) {
		bin_size -= 512;
		bload(ino, 1, progptr, bin_size);
		progptr += bin_size;
	}

	/* Should be smarter on the uzero: bank align the clearance */
	// zero all remaining process memory above the last block loaded.
	uzero((uint8_t *)progptr, top - progptr);

	udata.u_break = (int) progptr + bss;	//  Set initial break for program

	/* Turn off caught signals */
	memset(udata.u_sigvec, 0, sizeof(udata.u_sigvec));

	// place the arguments, environment and stack at the top of userspace memory,

	// Write back the arguments and the environment
	nargv = wargs(((char *) top - 2), abuf, &argc);
	nenvp = wargs((char *) (nargv), ebuf, NULL);

	// Fill in udata.u_name with Program invocation name
	uget((void *) ugetw(nargv), udata.u_name, 8);
	memcpy(udata.u_ptab->p_name, udata.u_name, 8);

	brelse(abuf);
	brelse(ebuf);

	// Shove argc and the address of argv just below envp
#ifdef CONFIG_CALL_R2L	/* Arguments are stacked the 'wrong' way around */
	uputw((uint16_t) nargv, nenvp - 1);
	uputw((uint16_t) argc, nenvp - 2);
#else
	uputw((uint16_t) nargv, nenvp - 1);
	uputw((uint16_t) argc, nenvp - 2);
#endif

	// Set stack pointer for the program
	udata.u_isp = nenvp - 2;

	// Start execution (never returns)
	doexec(PROGLOAD);

	// tidy up in various failure modes:
nogood3:
	brelse(abuf);
	brelse(ebuf);
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
