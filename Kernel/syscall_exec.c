#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>

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
	staticfast inoptr ino, emu_ino;
	staticfast unsigned char *buf;
	staticfast blkno_t blk;
	char **nargv;		/* In user space */
	char **nenvp;		/* In user space */
	staticfast struct s_argblk *abuf, *ebuf;
	int16_t (**sigp) ();
	int argc;
	uint16_t emu_size, emu_copy;
	uint8_t *progptr, *emu_ptr, *emu_base;
	staticfast uint16_t top;
	uint8_t c;
	uint16_t blocks;

	top = (uint16_t)ramtop;

	if (!(ino = n_open(name, NULLINOPTR)))
		return (-1);

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

	/* FIXME: In the execve case we may on some platforms have space
	   below PROGLOAD to clear... */

	/* We are definitely going to succeed with the exec,
	 * so we can start writing over the old program
	 */
	uput(buf, PROGLOAD, 512);	/* Move 1st Block to user bank */
	brelse(buf);

	c = ugetc(PROGLOAD);
	if (c != 0xC3)
		kprintf("Botched uput\n");

	/* At this point, we are committed to reading in and
	 * executing the program. */

	close_on_exec();

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
	progptr = PROGLOAD + 512;	// we copied the first block already

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
		doexec(PROGLOAD);

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
