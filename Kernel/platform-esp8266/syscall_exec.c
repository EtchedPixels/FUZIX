#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include "printf.h"

extern void platform_doexec(uaddr_t pc, void* sp);

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

	uaddr_t datatop = DATATOP;
	if (hdr.a_size)
		datatop = DATABASE + (hdr.a_size << 8);

	/* Does it fit? */

	uint32_t datasize = hdr.a_data + hdr.a_bss + USERSTACK;
	if ((hdr.a_text > CODELEN) || (datasize > DATALEN))
	{
		udata.u_error = ENOMEM;
		goto nogood2;
	}

	udata.u_ptab->p_status = P_NOSLEEP;

	/* If we made pagemap_realloc keep hold of some defined area we
	   could in theory just move the arguments up or down as part of
	   the process - that would save us all this hassle but replace it
	   with new hassle */

	/* Gather the arguments, and put them in temporary buffers. */
	struct s_argblk* abuf = (struct s_argblk *) tmpbuf();
	/* Put environment in another buffer. */
	struct s_argblk* ebuf = (struct s_argblk *) tmpbuf();

	/* Read args and environment from process memory */
	if (rargs(argv, abuf) || rargs(envp, ebuf))
		goto nogood3;	/* SN */

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
	udata.u_top = datatop;
	udata.u_ptab->p_top = datatop;

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
	 */

	/* Program code. */
	udata.u_base = (uint8_t*)CODEBASE + sizeof(hdr);
	udata.u_count = hdr.a_text - sizeof(hdr);
	udata.u_sysio = false;
	readi(ino, 0);
	if (udata.u_count != 0)
		goto nogood4;

	/* Data. */
	udata.u_base = (uint8_t*)DATABASE;
	udata.u_count = hdr.a_data;
	udata.u_sysio = false;
	readi(ino, 0);
	if (udata.u_count != 0)
		goto nogood4;

	/* Wipe the memory in the BSS. We don't wipe the memory above
	   that on 8bit boxes, but defer it to brk/sbrk() */
	uzero((uint8_t *)(DATABASE + hdr.a_data), hdr.a_bss);

	/* Set initial break for program */
	udata.u_break = (int)ALIGNUP(DATABASE + hdr.a_data + hdr.a_bss + USERSTACK);
	udata.u_texttop = CODEBASE + hdr.a_text;

	/* Turn off caught signals */
	memset(udata.u_sigvec, 0, sizeof(udata.u_sigvec));

	// place the arguments, environment and stack immediately above the bss.

	// Write back the arguments and the environment
	int argc;
	char** nargv = wargs((char *) udata.u_break - sizeof(uaddr_t), abuf, &argc);
	char** nenvp = wargs((char *) nargv, ebuf, NULL);

	// Fill in udata.u_name with program invocation name
	uget((void *) ugetp(nargv), udata.u_name, 8);
	memcpy(udata.u_ptab->p_name, udata.u_name, 8);

	tmpfree(abuf);
	tmpfree(ebuf);
	i_deref(ino);

	/* The LX106 stack pointer must be 16-aligned. */
	udata.u_sp = udata.u_isp = aligndown(nenvp - 3, 16);
	uputp((uint32_t) nenvp, udata.u_isp + 8);
	uputp((uint32_t) nargv, udata.u_isp + 4);
	uputp((uint32_t) argc, udata.u_isp + 0);

	/* Start execution (never returns) */
	udata.u_ptab->p_status = P_RUNNING;

	platform_doexec(CODEBASE + hdr.a_entry, udata.u_isp);
	panic("doexec returned\n");

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

/* vim: set ts=4 sw=4 et */

