#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <config.h>

#define BAD_ROOT_DEV 0xFFFF

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
		t++;
        }
}

void bufinit(void)
{
	bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

void fstabinit(void)
{
	struct mount *mp;

	for (mp = fs_tab; mp < fs_tab + NMOUNTS; ++mp) {
		mp->m_dev = NO_DEVICE;
	}
}

/* FIXME: pass remainder of boot argument to init, also word align */
void create_init(void)
{
	uint8_t *j;
	/* userspace: PROGLOAD +
               0    1    2    3    4   5  6  7  8  9  A  B  C */
	static const char arg[] =
	    { '/', 'i', 'n', 'i', 't', 0, 0, 1, 1, 0, 0, 0, 0 };

	init_process = ptab_alloc();
	udata.u_ptab = init_process;
	udata.u_top = PROGLOAD + 4096;	/* Plenty for the boot */
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
	udata.u_argn =  (arg_t)PROGLOAD;
	udata.u_argn1 = (arg_t)(PROGLOAD + 0x7); /* Arguments (just "/init") */
	udata.u_argn2 = (arg_t)(PROGLOAD + 0xb); /* Environment (none) */
}

/* to sensibly parse device names this needs to be platform-specific,
   this default version parses minor numbers only */
uint16_t default_bootdevice(unsigned char *s)
{
    unsigned int r = 0;

    /* skip spaces */
    while(*s == ' ')
        s++;

    while(true){
        if(*s >= '0' && *s <= '9'){
            r = (r*10) + (*s - '0');
        }else if(*s == '\r' || *s == '\n' || *s == 0){
            return r;
        }else
            return BAD_ROOT_DEV;
        s++;
    }
}

uint16_t get_root_dev(void)
{
	uint16_t rd = BAD_ROOT_DEV;
	char bootline[10];

	if (cmdline && *cmdline)
		rd = bootdevice(cmdline);

	while(rd == BAD_ROOT_DEV){
		kputs("bootdev: ");
		udata.u_base = (uint8_t*)&bootline;
		udata.u_sysio = 1;
		udata.u_count = sizeof(bootline)-1;
		udata.u_euid = 0;	/* Always begin as superuser */

		cdread(TTYDEV, O_RDONLY);	/* read root filesystem name from tty */
		rd = bootdevice(bootline);
	}

	return rd;
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

	/* Sign on messages */
	kprintf(
			"FUZIX version %s\n"
			"Copyright (c) 1988-2002 by H.F.Bower, D.Braun, S.Nitschke, H.Peraza\n"
			"Copyright (c) 1997-2001 by Arcady Schekochikhin, Adriano C. R. da Cunha\n"
			"Copyright (c) 2013-2015 Will Sowerbutts <will@sowerbutts.com>\n"
			"Copyright (c) 2014-2015 Alan Cox <alan@etchedpixels.co.uk>\nDevboot\n",
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
	/* Used as a stop marker to make compares fast on process
	   scheduling and the like */
	ptab_end = &ptab[maxproc];

	/* Parameters message */
	kprintf("%dkB total RAM, %dkB available to processes (%d processes max)\n", ramsize, procmem, maxproc);
	bufinit();
	fstabinit();
	pagemap_init();
	create_init();

	/* runtime configurable, defaults to build time setting */
	ticks_per_dsecond = TICKSPERSEC / 10;

	kputs("Enabling interrupts ... ");
	ei();
	kputs("ok.\n");

	/* initialise hardware devices */
	device_init();

	/* Mount the root device */
	root_dev = get_root_dev();
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

