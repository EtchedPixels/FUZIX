#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

static void close_on_exec(void)
{
	/* Keep the mask separate to stop SDCC generating crap code */
	uint16_t m = 1U << (UFTSIZE - 1);
	int8_t j;

	for (j = UFTSIZE - 1; j >= 0; --j) {
		if (udata.u_cloexec & m)
			doclose(j);
		m >>= 1;
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
#define name (uint8_t *)udata.u_argn
#define argv (char **)udata.u_argn1
#define envp (char **)udata.u_argn2

/*
 *	See exec.h
 */
static int header_ok(struct exec *pp)
{
	/* Executable ? */
	if (pp->a_magic != EXEC_MAGIC)
		return 0;
	/* Right CPU type ? */
	if (pp->a_cpu != sys_cpu)
		return 0;
	/* Compatible with this system ? */
	if ((pp->a_cpufeat & sys_cpu_feat) != pp->a_cpufeat)
		return 0;
	return 1;
}

arg_t _execve(void)
{
	/* We aren't re-entrant where this matters */
	struct exec hdr;
	staticfast inoptr ino;
	char **nargv;		/* In user space */
	char **nenvp;		/* In user space */
	struct s_argblk *abuf, *ebuf;
	int argc;
	uaddr_t progptr;
	uaddr_t progload;
	staticfast uaddr_t top;
	uaddr_t bin_size;	/* Will need to be bigger on some cpus */
	uaddr_t bss;

	top = ramtop;

	if (!(ino = n_open_lock(name, NULLINOPTR)))
		return (-1);

	if (!((getperm(ino) & OTH_EX) &&
	      (ino->c_node.i_mode & F_REG) &&
	      (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)))) {
		udata.u_error = EACCES;
		goto nogood;
	}

	setftime(ino, A_TIME);

	udata.u_offset = 0;
	udata.u_count = sizeof(struct exec);
	udata.u_base = (uint8_t *)&hdr;
	udata.u_sysio = true;

	readi(ino, 0);
	if (udata.u_done != sizeof(struct exec)) {
		udata.u_error = ENOEXEC;
		goto nogood;
	}

	if (!header_ok(&hdr)) {
		udata.u_error = ENOEXEC;
		goto nogood2;
	}

	if (pagemap_prepare(&hdr) < 0)
		goto nogood2;

	progload = hdr.a_base << 8;
	top = (hdr.a_base + hdr.a_size) << 8;

	/* For now assume no split I/D. We will need to revisit this and
	   pagemap_realloc when we add that so that the work is done in
	   pagemap_realloc and passed back somehow */

	/* top can overflow. We check below */
	bss = hdr.a_bss;

	bin_size = hdr.a_text + hdr.a_data;
	/* Does it fit ? */
	if (bin_size < hdr.a_text || top < progload || bin_size + bss < bin_size) {
		udata.u_error = ENOMEM;
		goto nogood2;
	}
	progptr = bin_size + 1024 + bss;
	if (bin_size < 64 || progload < PROGLOAD || top - progload < progptr || progptr < bin_size) {
		udata.u_error = ENOMEM;
		goto nogood2;
	}

	udata.u_ptab->p_status = P_NOSLEEP;

	/* If we made pagemap_realloc keep hold of some defined area we
	   could in theory just move the arguments up or down as part of
	   the process - that would save us all this hassle but replace it
	   with new hassle */

	/* Gather the arguments, and put them in temporary buffers. */
	abuf = (struct s_argblk *) tmpbuf();
	/* Put environment in another buffer. */
	ebuf = (struct s_argblk *) tmpbuf();

	/* Read args and environment from process memory */
	if (rargs(argv, abuf) || rargs(envp, ebuf))
		goto nogood3;	/* SN */

	/* This must be the last test as it makes changes if it works */
	/* This is only safe from deadlocks providing pagemap_realloc doesn't
	   sleep */
	if (pagemap_realloc(&hdr, top - MAPBASE))
		goto nogood3;

	/* From this point on we are commmited to the exec() completing */

	/* Core dump and ptrace permission logic */
#ifdef CONFIG_LEVEL_2
	/* Q: should uid == 0 mean we always allow core */
	if ((!(getperm(ino) & OTH_RD)) ||
		(ino->c_node.i_mode & (SET_UID | SET_GID)))
		udata.u_flags |= U_FLAG_NOCORE;
	else
		udata.u_flags &= ~U_FLAG_NOCORE;
#endif
	udata.u_ptab->p_top = top;

	/* setuid, setgid if executable requires it */
	if (ino->c_node.i_mode & SET_UID)
		udata.u_euid = ino->c_node.i_uid;

	if (ino->c_node.i_mode & SET_GID)
		udata.u_egid = ino->c_node.i_gid;

	/* FIXME: In the execve case we may on some platforms have space
	   below PROGLOAD to clear... */

	/*
	 * We place the stubs below the program in the hole left by the
	 * header. It's like the Linux VDSO except that it's not virtual
	 * not dynamic and not shared 8).
	 */
	uput(sys_stubs, (uint8_t *)progload, sizeof(struct exec));
	/* At this point, we are committed to reading in and
	 * executing the program. This call must not block. */

	close_on_exec();

	/*
	 *  Read in the rest of the program, block by block. We rely upon
	 *  the optimization path in readi to spot this is a big move to user
	 *  space and move it directly.
	 */

	progptr = progload + sizeof(struct exec);
	bin_size -= sizeof(struct exec);
	udata.u_base = (uint8_t *)progptr;		/* We copied the first block already */
	udata.u_count = bin_size;
	udata.u_sysio = false;
	readi(ino, 0);
	if (udata.u_done != bin_size)
		goto nogood4;
	progptr += bin_size;

	/* Wipe the memory in the BSS. We don't wipe the memory above
	   that on 8bit boxes, but defer it to brk/sbrk() */
	uzero((uint8_t *)progptr, bss);

	/* Set initial break for program */
	udata.u_break = (int)ALIGNUP(progptr + bss);

	/* Turn off caught signals */
	memset(udata.u_sigvec, 0, sizeof(udata.u_sigvec));

	// place the arguments, environment and stack at the top of userspace memory,

	// Write back the arguments and the environment
	nargv = wargs(((char *) top - 2), abuf, &argc);
	nenvp = wargs((char *) (nargv), ebuf, NULL);

	// Fill in udata.u_name with program invocation name
	uget((void *) ugetp(nargv), udata.u_name, 8);
	memcpy(udata.u_ptab->p_name, udata.u_name, 8);

	tmpfree(abuf);
	tmpfree(ebuf);
	i_deref(ino);

	/* Shove argc and the address of argv just below envp
	   FIXME: should flip them in crt0.S of app for R2L setups
	   so we can get rid of the ifdefs */
#ifdef CONFIG_CALL_R2L	/* Arguments are stacked the 'wrong' way around */
	uputp((uaddr_t) nargv, nenvp - 2);
	uputp((uaddr_t) argc, nenvp - 1);
#else
	uputp((uaddr_t) nargv, nenvp - 1);
	uputp((uaddr_t) argc, nenvp - 2);
#endif

	/* Set stack pointer for the program */
	udata.u_isp = nenvp - 2;

	/* Start execution (never returns) */
	udata.u_ptab->p_status = P_RUNNING;
	doexec(progload + hdr.a_entry);

	/* tidy up in various failure modes */
nogood4:
	/* Must not run userspace */
	ssig(udata.u_ptab, SIGKILL);
nogood3:
	udata.u_ptab->p_status = P_RUNNING;
	tmpfree(abuf);
	tmpfree(ebuf);
nogood2:
nogood:
	i_unlock_deref(ino);
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
	char *up = (char *)userspace_argv;
	uint8_t c;
	uint8_t *bufp;

	argbuf->a_argc = 0;	/* Store argc in argbuf */
	bufp = argbuf->a_buf;

	while ((ptr = (char *) ugetp(up)) != NULL) {
		up += sizeof(uptr_t);
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
	char *argv;		/* Address of users argv[], just below ptr */
	int argc, arglen;
	char *argbase;
	uint8_t *sptr;

	sptr = argbuf->a_buf;

	/* Move them into the users address space, at the very top */
	ptr -= (arglen = (int)ALIGNUP(argbuf->a_arglen));

	if (arglen) {
		uput(sptr, ptr, arglen);
	}

	/* Set argv to point below the argument strings */
	argc = argbuf->a_argc;
	argbase = argv = ptr - sizeof(uptr_t) * (argc + 1);

	if (cnt) {
		*cnt = argc;
	}

	/* Set each element of argv[] to point to its argument string */
	while (argc--) {
		uputp((uptr_t) ptr, argv);
		argv += sizeof(uptr_t);
		if (argc) {
			do
				++ptr;
			while (*sptr++);
		}
	}
	uputp(0, argv);		/*;;26Feb- Add Null Pointer to end of array */
	return (char **)argbase;
}

/*
 *	Stub the 32bit only allocator calls
 */

arg_t _memalloc(void)
{
	udata.u_error = ENOMEM;
	return -1;
}

arg_t _memfree(void)
{
	udata.u_error = ENOMEM;
	return -1;
}

#ifdef CONFIG_LEVEL_2

/*
 *	Core dump
 */

static struct coredump corehdr = {
	0xDEAD,
	0xC0DE,
	16,
};

uint8_t write_core_image(void)
{
	inoptr parent = NULLINODE;
	inoptr ino;

	udata.u_error = 0;

	/* FIXME: need to think more about the case sp is lala */
	if (uput("core", udata.u_syscall_sp - 5, 5))
		return 0;

	ino = n_open(udata.u_syscall_sp - 5, &parent);
	if (ino) {
		i_deref(parent);
		return 0;
	}
	if (parent) {
		i_lock(parent);
		if (ino = newfile(parent, "core")) {
			ino->c_node.i_mode = F_REG | 0400;
			setftime(ino, A_TIME | M_TIME | C_TIME);
			wr_inode(ino);
			f_trunc(ino);

			/* FIXME: need to add some arch specific header bits, and
			   also pull stuff like the true sp and registers out of
			   the return stack properly */

			corehdr.ch_base = MAPBASE;
			corehdr.ch_break = udata.u_break;
			corehdr.ch_sp = udata.u_syscall_sp;
			corehdr.ch_top = PROGTOP;
			udata.u_offset = 0;
			udata.u_base = (uint8_t *)&corehdr;
			udata.u_sysio = true;
			udata.u_count = sizeof(corehdr);
			writei(ino, 0);
			udata.u_sysio = false;
			udata.u_base = MAPBASE;
			udata.u_count = udata.u_break - MAPBASE;
			writei(ino, 0);
			udata.u_base = udata.u_sp;
			udata.u_count = PROGTOP - (uint16_t)udata.u_sp;
			writei(ino, 0);
			i_unlock_deref(ino);
			return W_COREDUMP;
		}
	}
	return 0;
}
#endif
