#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include "printf.h"
#include "globals.h"

extern void plt_doexec(uaddr_t pc, void* sp);

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
#define argv (uint8_t **)udata.u_argn1
#define envp (uint8_t **)udata.u_argn2

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
	int argc;
	uint8_t **nargv, **nenvp;
	struct s_argblk* abuf = NULL;
	struct s_argblk* ebuf = NULL;

	if (!(ino = n_open_lock(name, NULLINOPTR)))
		return (-1);

	if (!((getperm(ino) & OTH_EX) &&
		  (ino->c_node.i_mode & F_REG) &&
		  (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX))))
		goto eacces;

	setftime(ino, A_TIME);

	udata.u_offset = 0;
	udata.u_count = sizeof(struct exec);
	udata.u_base = (uint8_t *)&hdr;
	udata.u_sysio = true;

	readi(ino, 0);
	if (udata.u_done != sizeof(struct exec))
		goto enoexec;

	if (!header_ok(&hdr))
		goto enoexec;

	/* Does it fit? */

	uint32_t codesize = hdr.a_text;
	uint32_t datasize = (uint32_t)alignup(hdr.a_data + hdr.a_bss + USERSTACK, 16);
	if ((codesize > CODELEN) || ((PROGLOAD + datasize) > DATATOP))
		goto enomem;

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
		goto enomem;

	/* At this point we should call pagemap_realloc(), for this to work on a
     * variable-sized process system. This must be the last test as it makes
     * changes if it works. */

    if (pagemap_realloc_code_and_data(codesize, datasize))
		goto enomem;
	
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

	/* setuid, setgid if executable requires it */
	if (ino->c_node.i_mode & SET_UID)
		udata.u_euid = ino->c_node.i_uid;

	if (ino->c_node.i_mode & SET_GID)
		udata.u_egid = ino->c_node.i_gid;

	/* At this point, we are committed to reading in and
	 * executing the program. This call must not block. */

	close_on_exec();

	/*
	 *  Read in the rest of the program, block by block. We rely upon
	 *  the optimization path in readi to spot this is a big move to user
	 *  space and move it directly.
	 *
	 * We can skip the use of valaddr() as we use fixed ranges and already
	 * did the size checks.
	 */

	/* Program code. */
	udata.u_base = (uint8_t *)CODEBASE + sizeof(hdr);
	udata.u_count = hdr.a_text - sizeof(hdr);
	udata.u_sysio = false;
	readi(ino, 0);
	if (udata.u_count != 0)
		goto fatal;

	/* Data. */
	udata.u_base = (uint8_t*)PROGLOAD;
	udata.u_count = hdr.a_data;
	udata.u_sysio = false;
	readi(ino, 0);
	if (udata.u_count != 0)
		goto fatal;

	/* Wipe the memory in the BSS. We don't wipe the memory above
	   that on 8bit boxes, but defer it to brk/sbrk() */
	uzero((uint8_t *)(PROGLOAD + hdr.a_data), hdr.a_bss);

	/* Set initial break for program */
	udata.u_break = udata.u_top;

	/* Turn off caught signals */
	memset(udata.u_sigvec, 0, sizeof(udata.u_sigvec));

	// place the arguments, environment and stack at the top of userspace memory,

	// Write back the arguments and the environment
	nargv = wargs((uint8_t *) udata.u_top, abuf, &argc);
	nenvp = wargs((uint8_t *) nargv, ebuf, NULL);

	// Fill in udata.u_name with program invocation name
	uget((void *) ugetp(nargv), udata.u_name, 8);
	memcpy(udata.u_ptab->p_name, udata.u_name, 8);

	tmpfree(abuf);
	tmpfree(ebuf);
	i_deref(ino);

	/* The LX106 stack pointer must be 16-aligned. */
	udata.u_isp = aligndown(nenvp - 3, 16);
	uputp((uint32_t) nenvp, udata.u_isp + 8);
	uputp((uint32_t) nargv, udata.u_isp + 4);
	uputp((uint32_t) argc, udata.u_isp + 0);

	/* Start execution (never returns) */
	udata.u_ptab->p_status = P_RUNNING;

	plt_doexec(CODEBASE + hdr.a_entry, udata.u_isp);
	panic("doexec returned\n");

	/* tidy up in various failure modes */
fatal:
	/* Must not run userspace */
	ssig(udata.u_ptab, SIGKILL);
	udata.u_ptab->p_status = P_RUNNING;
error:
	if (abuf)
		tmpfree(abuf);
	if (ebuf)
		tmpfree(ebuf);
	i_unlock_deref(ino);
	return (-1);

enomem:
	udata.u_error = ENOMEM;
	goto error;

eacces:
	udata.u_error = EACCES;
	goto error;

enoexec:
	udata.u_error = ENOEXEC;
	goto error;
}

#undef name
#undef argv
#undef envp

/*
 *      Stub the 32bit only allocator calls
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

