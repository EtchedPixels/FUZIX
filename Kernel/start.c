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

static const struct termios ttydflt = {
	BRKINT | ICRNL,
	OPOST | ONLCR,
	CS8 | B9600 | CREAD | HUPCL,
	ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN,
	{CTRL('D'), 0, 127, CTRL('C'),
	 CTRL('U'), CTRL('\\'), CTRL('Q'), CTRL('S'),
	 CTRL('Z'), CTRL('Y'), CTRL('V'), CTRL('O')
	 }
};

void tty_init(void) {
        struct tty *t = &ttydata[1];
        int i;
        for(i = 1; i <= NUM_DEV_TTY; i++) {
		memcpy(&t->termios, &ttydflt, sizeof(struct termios));
		kprintf("%x: %x\n", &t->termios, t->termios.c_lflag);
		t++;
        }
}

void bufinit(void)
{
	bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = false;
	}
}


void create_init(void)
{
	uint8_t *j;

	/* userspace (offset from PROGBASE): 0x0000+ 0   1   2   3   4   5   6   7   8   9   A   B   C */
	/* FIXME: big-endian only */
	const char arg[] =
	    { '/', 'i', 'n', 'i', 't', 0, 0, (uint16_t)(PROGBASE + 1) & 0xFF, (uint16_t)(PROGBASE + 1) >> 8, 0, 0, 0, 0 };


	init_process = ptab_alloc();
	kprintf("init PID is %d\r\n", init_process);
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
	uput(arg, (void *)PROGLOAD, sizeof(arg));
	/* Poke in argv[0] - FIXME: Endianisms...  */
	uputw(PROGLOAD+1 , (void *)(PROGLOAD + 7));

	/* Set up things to look like the process is calling _execve() */
	udata.u_argn = (uint16_t) PROGBASE;
	udata.u_argn1 = (uint16_t)(PROGBASE + 0x07);	/* Arguments (just "/init") */
	udata.u_argn2 = (uint16_t)(PROGBASE + 0x0b);	/* Environment (none) */

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
	kprintf(
	 "FUZIX version %s\n"
	 "Copyright (c) 1988-2002 by H.F.Bower, D.Braun, S.Nitschke, H.Peraza\n"
	 "Copyright (C) 1997-2001 by Arcady Schekochikhin, Adriano C. R. da Cunha\n"
	 "Copyright (c) 2013 Will Sowerbutts <will@sowerbutts.com>\n"
	 "Copyright (c) 2014 Alan Cox <alan@etchedpixels.co.uk>\nDevboot\n",
	                                         uname_str);

#ifndef SWAPDEV
#ifdef PROC_SIZE
	maxproc = procmem / PROC_SIZE;
	/* Check we don't exceed the process table size limit */
	if (maxproc > PTABSIZE) {
		kprintf("WARNING: Increase PTABSIZE to %d to use available RAM\n",
		                maxproc);
		maxproc = PTABSIZE;
	}
#else
	maxproc = PTABSIZE;
#endif
#else
	maxproc = PTABSIZE;
#endif
	/* Parameters message */
	kprintf("%dkB total RAM, %dkB available to processes (%d processes max)\n", ramsize, procmem, maxproc);
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

