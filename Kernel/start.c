#include <kernel.h>
#include <version.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>

/*
 *	Put nothing here that cannot be discarded. We will eventually
 *	make the entire of this disappear after the initial _execve
 */

void create_init(void)
{
	uint8_t *j;
	/* userspace: 0x100+ 0   1   2   3   4   5   6   7   8   9   A   B   C */
	const char arg[] =
	    { '/', 'i', 'n', 'i', 't', 0, 0, 1, 1, 0, 0, 0, 0 };

	init_process = ptab_alloc();
	udata.u_ptab = init_process;
	udata.u_top = 4096;	/* Plenty for the boot */
	map_init();
	newproc(init_process);

	init_process->p_status = P_RUNNING;

	/* wipe file table */
	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j) {
		*j = NO_FILE;
	}
	/* Poke the execve arguments into user data space so _execve() can read them back */
	uput(arg, PROGBASE, sizeof(arg));

	/* Set up things to look like the process is calling _execve() */
	udata.u_argn = (uint16_t) PROGBASE;
	/* FIXME - should be relative to PROGBASE... */
	udata.u_argn1 = 0x107;	/* Arguments (just "/init") */
	udata.u_argn2 = 0x10b;	/* Environment (none) */
}

void fuzix_main(void)
{
	/* setup state */
	inint = false;
	udata.u_insys = true;

	ramtop = PROGTOP;

	tty_init();

	if (d_open(TTYDEV, 0) != 0)
		panic("no tty");

	/* Sign on messages (stashed in a buffer so we can bin them */
	kprintf((char *)bufpool[0].bf_data, uname_str);

#ifndef SWAPDEV
#ifdef PROC_SIZE
	maxproc = procmem / PROC_SIZE;
	/* Check we don't exceed the process table size limit */
	if (maxproc > PTABSIZE) {
		kprintf((char *)bufpool[1].bf_data, maxproc);
		maxproc = PTABSIZE;
	}
#else
	maxproc = PTABSIZE;
#endif
#else
	maxproc = PTABSIZE;
#endif
	/* Parameters message */
	kprintf((char *)bufpool[2].bf_data, ramsize, procmem, maxproc);
	/* Now blow away the strings */
	bufinit();
	pagemap_init();

	create_init();
        kprintf("%x:%x\n", udata.u_page, udata.u_page2);
        kprintf("%x:%x\n", udata.u_ptab->p_page, udata.u_ptab->p_page2);
	kputs("Enabling interrupts ... ");
        ei();
	kputs("ok.\n");

	/* initialise hardware devices */
	device_init();

	root_dev = DEFAULT_ROOT;
	if (cmdline && *cmdline) {
		while (*cmdline == ' ')
			++cmdline;
		root_dev = *cmdline - '0';
	} else {
		kputs("bootdev: ");
		udata.u_base = bootline;
		udata.u_sysio = 1;
		udata.u_count = 2;
		udata.u_euid = 0;	/* Always begin as superuser */

		cdread(TTYDEV, O_RDONLY);	/* read root filesystem name from tty */
		if (*bootline >= '0')
			root_dev = *bootline - '0';
	}

	/* Mount the root device */
	kprintf("Mounting root fs (root_dev=%d): ", root_dev);

	if (fmount(root_dev, NULLINODE, 0))
		panic("no filesys");
	root = i_open(root_dev, ROOTINODE);
	if (!root)
		panic("no root");

	kputs("OK\n");

	i_ref(udata.u_cwd = root);
	i_ref(udata.u_root = root);
	rdtime32(&udata.u_time);
	exec_or_die();
}

#ifdef CONFIG_IDUMP

void idump(void)
{
	inoptr ip;
	ptptr pp;
	extern struct cinode i_tab[];

	kprintf("Err %d root %d\n", udata.u_error, root - i_tab);
	kputs("\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY\n");

	for (ip = i_tab; ip < i_tab + ITABSIZE; ++ip) {
		kprintf("%d\t%d\t%d\t%u\t0%o\t",
			ip - i_tab, ip->c_magic, ip->c_dev, ip->c_num,
			ip->c_node.i_mode);
		kprintf("%d\t%d\t%d\t%d\n",	/* line split for compiler */
			ip->c_node.i_nlink, ip->c_node.i_addr[0],
			ip->c_refs, ip->c_dirty);
		if (!ip->c_magic)
			break;
	}

	kputs
	    ("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\tCHILD\n");
	for (pp = ptab; pp < ptab + PTABSIZE /*maxproc */ ; ++pp) {
		if (pp->p_status == P_EMPTY)
			continue;
		kprintf("%d\t%d\t0x%x\t%d\t",
			pp - ptab, pp->p_status, pp->p_wait, pp->p_pid);
		kprintf("%d\t%d\t0x%lx\t0x%lx\n",
			pp->p_pptr - ptab, pp->p_alarm, pp->p_pending,
			pp->p_ignored);
	}

	bufdump();

	kprintf("insys %d ptab %d call %d cwd %d sp 0x%x\n",
		udata.u_insys, udata.u_ptab - ptab, udata.u_callno,
		udata.u_cwd - i_tab, udata.u_sp);
}

#endif
