#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

/* We don't share this routine between the exec routines as we optimise the
   8bit one differently */
static void close_on_exec(void)
{
	/* Keep the mask separate to stop SDCC generating crap code */
	register uint16_t m = 1U << (UFTSIZE - 1);
	register int_fast8_t j;

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
#define argv (uint8_t **)udata.u_argn1
#define envp (uint8_t **)udata.u_argn2

/*
 *	See exec.h
 */
static int header_ok(register struct exec *pp)
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
	staticfast struct exec hdr;
	staticfast inoptr ino;
	uint8_t **nargv;		/* In user space */
	uint8_t **nenvp;		/* In user space */
	struct s_argblk *abuf, *ebuf;
	int argc;
	uaddr_t progptr;
	uaddr_t progload;
	staticfast uaddr_t top;
	uaddr_t bin_size;	/* Will need to be bigger on some cpus */
	uaddr_t bss;
	uint_fast8_t mflags;

	top = ramtop;

	if (!(ino = n_open_lock(name, NULLINOPTR)))
		return (-1);

	if (!((getperm(ino) & OTH_EX) &&
	      (ino->c_node.i_mode & F_REG) &&
	      (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)))) {
		udata.u_error = EACCES;
		goto nogood;
	}

	mflags = fs_tab[ino->c_super].m_flags;
	if (mflags & MS_NOEXEC) {
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
#ifdef DP_SIZE
	if (hdr.a_zp > DP_SIZE) {
		udata.u_error = ENOMEM;
		goto nogood2;
	}
#endif
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

#ifdef CONFIG_PLATFORM_UDMA
	plt_udma_kill(udata.u_ptab);
#endif
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
	udata.u_top = top;
	udata.u_ptab->p_top = top;

	if (!(mflags & MS_NOSUID)) {
		/* setuid, setgid if executable requires it */
		if (ino->c_node.i_mode & SET_UID)
			udata.u_euid = ino->c_node.i_uid;
		if (ino->c_node.i_mode & SET_GID)
			udata.u_egid = ino->c_node.i_gid;
	}

	/* FIXME: In the execve case we may on some platforms have space
	   below PROGLOAD to clear... */

	udata.u_codebase = progload;

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

	/* Should not be possible */
	if (valaddr_r(udata.u_base, udata.u_count) != udata.u_count)
		goto nogood4;
	readi(ino, 0);
	if (udata.u_done != bin_size)
		goto nogood4;
	progptr += bin_size;

	/* Wipe the memory in the BSS. We don't wipe the memory above
	   that on 8bit boxes, but defer it to brk/sbrk() */
	uzero((uint8_t *)progptr, bss);

	/* Wipe zero page/direct page spaces if present */
#ifdef DP_SIZE
	uzero((uint8_t *)DP_BASE, DP_SIZE);
#endif

	/* Set initial break for program */
	udata.u_break = (int)ALIGNUP(progptr + bss);

	/* Turn off caught signals */
	memset(udata.u_sigvec, 0, sizeof(udata.u_sigvec));

	// place the arguments, environment and stack at the top of userspace memory,

	// Write back the arguments and the environment
	nargv = wargs(((uint8_t *) top - 2), abuf, &argc);
	nenvp = wargs((uint8_t *) (nargv), ebuf, NULL);

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

