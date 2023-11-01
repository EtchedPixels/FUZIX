/*
 *	Common routines for syscall handling
 */

#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

/* This is hardcoded as we use disk buffers as scratch for the arguments
   on most platforms */
#define ARGBUF_SIZE	BLKSIZE

bool rargs(uint8_t **userspace_argv, struct s_argblk * argbuf)
{
	uint8_t *ptr;		/* Address of base of arg strings in user space */
	register uint8_t *up = (uint8_t *)userspace_argv;
	register uint8_t c;
	register uint8_t *bufp;
	uint8_t *ep = argbuf->a_buf + ARGBUF_SIZE - 12;

	argbuf->a_argc = 0;	/* Store argc in argbuf */
	bufp = argbuf->a_buf;

	while ((ptr = (uint8_t *) ugetp(up)) != NULL) {
		up += sizeof(uptr_t);
		++(argbuf->a_argc);	/* Store argc in argbuf. */
		do {
			*bufp++ = c = ugetc(ptr++);
			if (bufp > ep) {
				udata.u_error = E2BIG;
				return true;	// failed
			}
		}
		while (c);
	}
	argbuf->a_arglen = bufp - (uint8_t *)argbuf->a_buf;	/* Store total string size. */
	argbuf->a_arglen = (size_t)ALIGNUP(argbuf->a_arglen);
	return false;		// success
}


uint8_t **wargs(uint8_t *ptr, struct s_argblk *argbuf, int *cnt)	// ptr is in userspace
{
	register uint8_t *argv;		/* Address of users argv[], just below ptr */
	register unsigned int argc, arglen;
	uint8_t *argbase;
	register uint8_t *sptr;

	sptr = argbuf->a_buf;

	/* Move them into the users address space, at the very top */
	ptr -= (arglen = (int)ALIGNUP(argbuf->a_arglen));

	if (arglen) {
		_uput(sptr, ptr, arglen);
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
	uputp(0, argv);
	return (uint8_t **)argbase;
}

#ifdef CONFIG_LEVEL_2

/*
 *	Core dump: FIXME write a proper core header and usable image. Right now this
 *	is basically a dummy
 */

static struct coredump corehdr = {
	0xDEAD,
	0xC0DE,
	sizeof(uarg_t) * 8,	/* user integer size in bits */
};

static struct coremem memhdr = {
	COREHDR_MEM
};

void coredump_memory(inoptr ino, uaddr_t base, usize_t len, uint16_t flags)
{
	memhdr.mh_base = base;
	memhdr.mh_len = len;
	memhdr.mh_flags = flags;
	udata.u_base = (uint8_t *)&memhdr;
	udata.u_sysio = true;
	udata.u_count = sizeof(memhdr);
	writei(ino, 0);
	udata.u_base = (uint8_t *)base;
	udata.u_sysio = false;
	udata.u_count = len;
	writei(ino, 1);
}

uint8_t write_core_image(void)
{
	inoptr parent;
	register inoptr ino;

	udata.u_error = 0;

	/* FIXME: need to think more about the case sp is lala */
	if (uput("core", (void *)(udata.u_syscall_sp - 5), 5))
		return 0;

	ino = n_open((uint8_t *)udata.u_syscall_sp - 5, &parent);
	if (ino) {
		i_deref(parent);
		return 0;
	}
	if (parent) {
		i_lock(parent);
		if ((ino = newfile(parent, (uint8_t *)"core")) != NULL) {
			ino->c_node.i_mode = F_REG | 0400;
			setftime(ino, A_TIME | M_TIME | C_TIME);
			wr_inode(ino);
			f_trunc(ino);
			/* Write the header */
			corehdr.ch_base = (uptr_t)udata.u_codebase;
			corehdr.ch_break = udata.u_break;
			corehdr.ch_sp = udata.u_syscall_sp;
#ifdef PROGTOP
			corehdr.ch_top = PROGTOP;
#else
			corehdr.ch_top = udata.u_top;
#endif
			udata.u_offset = 0;
			udata.u_base = (uint8_t *)&corehdr;
			udata.u_sysio = true;
			udata.u_count = sizeof(corehdr);
			writei(ino, 0);
			/* Ask the architecture to dump the user registers */
//TODO			coredump_user_registers(ino);
			/* Ask the memory manager to dump the memory map */
			coredump_image(ino);
			i_unlock_deref(ino);
			i_unlock_deref(parent);
			return W_COREDUMP;
		}
		i_unlock_deref(parent);
	}
	return 0;
}
#endif
