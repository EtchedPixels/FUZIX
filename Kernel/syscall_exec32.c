/*
 *	Implement binary loading for 32bit platforms. We use the ucLinux binflat
 *	format with a simple magic number tweak to avoid confusion with ucLinux
 */

#include <kernel.h>
#include <kernel32.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

#define ARGBUF_SIZE	2048

struct binfmt_flat {
	uint8_t magic[4];
	uint32_t rev;
	uint32_t entry;
	uint32_t data_start;
	uint32_t data_end;
	uint32_t bss_end;
	uint32_t stack_size;
	uint32_t reloc_start;
	uint32_t reloc_count;
	uint32_t flags;
	uint32_t filler[6];
};

static int bload(inoptr i, uint16_t bl, void *base, uint32_t len)
{
	blkno_t blk;
	while(len) {
		uint16_t cp = min(len, 512);
		blk = bmap(i, bl, 1);
		if (blk == NULLBLK)
			uzero(base, 512);
		else {
			udata.u_offset = (off_t)blk << 9;
			udata.u_count = 512;
			udata.u_base = base;
			if (cdread(i->c_dev, 0) < 0) {
				kputs("bload failed.\n");
				return -1;
			}
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

static int valid_hdr(inoptr ino, struct binfmt_flat *bf)
{
	if (bf->rev != 4)
		return 0;
	if (bf->entry >= bf->data_start)
		return 0;
	if (bf->data_start > bf->data_end)
		return 0;
	if (bf->data_end < bf->bss_end)
		return 0;
	if (bf->bss_end + bf->stack_size < bf->bss_end)
		return 0;
	if (bf->data_end > ino->c_node.i_size)
		return 0;
	if (bf->bss_end - bf->data_end < 4 * bf->reloc_count)
		bf->bss_end = bf->data_end + 4 * bf->reloc_count;
	if (bf->reloc_start + bf->reloc_count * 4 > ino->c_node.i_size ||
		bf->reloc_start + bf->reloc_count * 4 < bf->reloc_start)
		return 0;
	if (bf->flags != 1)
		return 0;
	return 1;
}

/* For now we load the binary in one block, including code/data/bss. We can
   look at better formats, split binaries etc later maybe */
static void relocate(struct binfmt_flat *bf, void *progbase, uint32_t size)
{
	uint32_t *rp = progbase + bf->reloc_start;
	uint32_t n = bf->reloc_count;
	while (n--) {
		uint32_t v = *rp++;
		if (v < size && !(v&1))	/* Revisit for non 68K */
			*((uint32_t *)(rp + v)) += (uint32_t)progbase;
	}
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

arg_t _execve(void)
{
	struct binfmt_flat *binflat;
	inoptr ino;
	unsigned char *buf;
	char **nargv;		/* In user space */
	char **nenvp;		/* In user space */
	struct s_argblk *abuf, *ebuf;
	int argc;
	uint32_t bin_size;	/* Will need to be bigger on some cpus */
	void *progbase, *top;
	uaddr_t go;

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
	binflat = (struct binfmt_flat *)buf;

	/* Hard coded for our 68K format. We don't quite use the ucLinux
	   names, we don't want to load a ucLinux binary in error! */
	if (buf == NULL || memcmp(buf, "bF68", 4) ||
		!valid_hdr(ino, binflat)) {
		udata.u_error = ENOEXEC;
		goto nogood2;
	}

	/* Memory needed */
	bin_size = binflat->bss_end + binflat->stack_size;

	/* Gather the arguments, and put them in temporary buffers. */
	abuf = (struct s_argblk *) kmalloc(ARGBUF_SIZE);
	if (abuf == NULL) {
		udata.u_error = ENOMEM;
		goto nogood2;
	}
	/* Put environment in another buffer. */
	ebuf = (struct s_argblk *) kmalloc(ARGBUF_SIZE);
	if (ebuf == NULL) {
		kfree(abuf);
		udata.u_error = ENOMEM;
		goto nogood2;
	}

	/* Read args and environment from process memory */
	if (rargs(argv, abuf) || rargs(envp, ebuf))
		goto nogood3;

	/* This must be the last test as it makes changes if it works */
	if (pagemap_realloc(bin_size))
		goto nogood3;

	/* From this point on we are commmited to the exec() completing */

	/* setuid, setgid if executable requires it */
	if (ino->c_node.i_mode & SET_UID)
		udata.u_euid = ino->c_node.i_uid;

	if (ino->c_node.i_mode & SET_GID)
		udata.u_egid = ino->c_node.i_gid;

	/* We are definitely going to succeed with the exec,
	 * so we can start writing over the old program
	 */
	
	progbase = pagemap_base();
	top = progbase + bin_size;

	uput(buf, (uint8_t *)progbase, 512);	/* Move 1st Block to user bank */

	/* At this point, we are committed to reading in and
	 * executing the program. */

	close_on_exec();

	/*
	 *  Read in the rest of the program, block by block
	 *  We use bufdiscard so that we load the entire app through the
	 *  same buffer to avoid cycling our small cache on this. Indirect blocks
	 *  will still be cached. - Hat tip to Steve Hosgood's OMU for that trick
	 */

	/* Compute this once otherwise each loop we must recalculate this
	   as the compiler isn't entitled to assume the loop didn't change it */

	bin_size = binflat->reloc_start + 4 * binflat->reloc_count;
	if (bin_size > 512)
		bload(ino, 1, progbase + 512, bin_size - 512);
	
	go = (uint32_t)progbase + binflat->entry;

	relocate(binflat, progbase, bin_size);
	/* This may wipe the relocations */	
	uzero(progbase + binflat->data_end, 
		binflat->bss_end - binflat->data_end + binflat->stack_size);

	brelse(buf);

	/* brk eats into the stack allocation */
	udata.u_break = (uaddr_t)(progbase + binflat->bss_end);

	/* Turn off caught signals */
	memset(udata.u_sigvec, 0, sizeof(udata.u_sigvec));

	/* place the arguments, environment and stack at the top of userspace memory. */

	/* Write back the arguments and the environment */
	nargv = wargs(((char *) top - 4), abuf, &argc);
	nenvp = wargs((char *) (nargv), ebuf, NULL);

	/* Fill in udata.u_name with Program invocation name */
	uget((void *) ugetl(nargv, NULL), udata.u_name, 8);
	memcpy(udata.u_ptab->p_name, udata.u_name, 8);

	kfree(abuf);
	kfree(ebuf);
	i_deref(ino);

	/* Shove argc and the address of argv just below envp */
	uputl((uint32_t) nargv, nenvp - 1);
	uputl((uint32_t) argc, nenvp - 2);

	// Set stack pointer for the program
	udata.u_isp = nenvp - 4;

	// Start execution (never returns)
	doexec(go);

	// tidy up in various failure modes:
nogood3:
	kfree(abuf);
	kfree(ebuf);
nogood2:
	brelse(buf);
nogood:
	i_deref(ino);
	return (-1);
}

#undef name
#undef argv
#undef envp

/* TODO        max (1024) 512 bytes for argv
 *             and max 512 bytes for environ
 */

bool rargs(char **userspace_argv, struct s_argblk * argbuf)
{
	char *ptr;		/* Address of base of arg strings in user space */
	uint8_t c;
	uint8_t *bufp;
	int err;

	argbuf->a_argc = 0;	/* Store argc in argbuf */
	bufp = argbuf->a_buf;

	while ((ptr = (char *) ugetl(userspace_argv++, &err)) != NULL) {
		if (err)
			return true;
		++(argbuf->a_argc);	/* Store argc in argbuf. */
		do {
			*bufp++ = c = ugetc(ptr++);
			if (bufp > argbuf->a_buf + ARGBUF_SIZE - 12) {
				udata.u_error = E2BIG;
				return true;	// failed
			}
		}
		while (c);
	}
	/*Store total string size. */
	argbuf->a_arglen = bufp - (uint8_t *)argbuf->a_buf;
	/* Success */
	return false;
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
		uputl((uint32_t) ptr, argv++);
		if (argc) {
			do
				++ptr;
			while (*sptr++);
		}
	}
	uputl(0, argv);
	return ((char **) argbase);
}

