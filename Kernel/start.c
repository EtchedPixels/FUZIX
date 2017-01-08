#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <config.h>

#define BAD_ROOT_DEV 0xFFFF

static uint8_t ro;

/*
 *	Put nothing here that cannot be discarded. We make the entirety
 *	of this disappear after the initial _execve.
 */

#ifndef TTY_INIT_BAUD
#define TTY_INIT_BAUD B9600
#endif

static const struct termios ttydflt = {
	BRKINT | ICRNL,
	OPOST | ONLCR,
	CS8 | TTY_INIT_BAUD | CREAD | HUPCL,
	ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN,
	{CTRL('D'), 0, CTRL('H'), CTRL('C'),
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

	for (bp = bufpool; bp < bufpool_end; ++bp) {
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

/* FIXME: pass remainder of boot argument to init */
/* Remember two things when modifying this code
   1. Some processors need 2 byte alignment or better of arguments. We
      lay it out for 4
   2. We are going to end up with cases where user and kernel pointer
      size differ due to memory models etc. We use uputp and we allow
      room for the pointers to be bigger than kernel */

static uaddr_t progptr, old_progptr;
static uaddr_t argptr, old_argptr;

void add_argument(const char *s)
{
	int l = strlen(s) + 1;
	uput(s, (void *)progptr, l);
	uputp(progptr, (void *)argptr);
	progptr += ((l + 3) & ~3);
	argptr += sizeof(uptr_t);
}

void create_init(void)
{
	uint8_t *j;

	init_process = ptab_alloc();
	udata.u_ptab = init_process;
	udata.u_top = PROGLOAD + 4096;	/* Plenty for the boot */
	init_process->p_top = udata.u_top;
	map_init();
	newproc(init_process);

	udata.u_insys = 1;

	init_process->p_status = P_RUNNING;

	/* wipe file table */
	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j) {
		*j = NO_FILE;
	}
	/* Poke the execve arguments into user data space so _execve() can read them back */
	argptr = PROGLOAD;
	progptr = PROGLOAD + 2048;

	uzero((void *)progptr, 32);
	add_argument("/init");
}

void complete_init(void)
{
	/* Terminate argv, also use this as the env ptr */
	uputp(0, (void *)argptr);
	/* Set up things to look like the process is calling _execve() */
	udata.u_argn =  (arg_t)PROGLOAD + 2048; /* "/init" */
	udata.u_argn1 = (arg_t)PROGLOAD; /* Arguments */
	udata.u_argn2 = (arg_t)argptr; /* Environment (none) */

#ifdef CONFIG_LEVEL_2
	init_process->p_session = 1;
#endif
	init_process->p_pgrp = 1;
}

#ifndef BOOTDEVICE

/* Parse boot device name, based on platform defined BOOTDEVICENAMES string.
 *
 * This string is a list of device driver names delimited by commas. Device
 * driver names should be listed in the same order as entries in dev_tab.
 * Unbootable slots should be listed with an empty name. The position in the
 * list of names specifies the top 8 bits of the minor number.
 *
 * Names which end in a # character expect a letter suffix which specifies bits
 * 4--7 of the minor number.
 *
 * All names can be followed by an index number which is added to the minor
 * number. The user can provide only this index number, in which case it
 * specifies the full minor number.
 *
 * Some example BOOTDEVICENAMES:
 * "hd#,fd"
 *    hda   gives 0x0000
 *    hda1  gives 0x0001
 *    hdc   gives 0x0020
 *    hdc3  gives 0x0023
 *    fd0   gives 0x0100
 *    fd15  gives 0x010F
 *    17    gives 0x0011
 *
 * "fd,hd#,,ram"  (slot 2 in dev_tab is the tty device which is unbootable)
 *    fd0   gives 0x0000
 *    fd1   gives 0x0001
 *    hda1  gives 0x0101
 *    hdc3  gives 0x0123
 *    ram7  gives 0x0307
 *    17    gives 0x0011
 */

#ifndef BOOTDEVICENAMES
#define BOOTDEVICENAMES "" /* numeric parsing only */
#endif

static uint8_t system_param(char *p)
{
	if (*p == 'r' && p[2] == 0) {
		if (p[1] == 'o') {
			ro = MS_RDONLY;
			return 1;
		} else if (p[1] == 'w') {
			ro = 0;
			return 1;
		}
	}
	/* FIXME: Parse init=path ?? */
	return platform_param(p);
}

/* Parse other arguments */
void parse_args(char *p)
{
	char *s;
	while(*p) {
		while(*p == ' ' || *p == '\n')
			p++;
		s = p;
		while(*p && *p != ' ' && *p != '\n')
			p++;
		if(*p)
			*p++=0;
		if (!system_param(s))
			add_argument(s);

	}
}

uint16_t bootdevice(char *devname)
{
	bool match = true;
	unsigned int b = 0, n = 0;
	char *p;
	const uint8_t *bdn = (const uint8_t *)BOOTDEVICENAMES;
	uint8_t c, pc;

	/* skip spaces at start of string */
	while(*devname == ' '){
		devname++;
	}

	p = devname;

	/* first we try to the match device name */
	while(true){
		pc = *p;
		if(pc >= 'A' && pc <= 'Z')
			pc |= 0x20; /* lower case */
		c = *bdn;

		if(!c){
			/* end of device names string */
			break;
		}else if(c == ','){
			/* next device driver */
			if(match == true && p != devname)
				break;
			p = devname;
			b += 0x100;
			match = true;
			bdn++;
			continue;
		}else if(match && c == '#'){
			/* parse device drive letter */
			if(pc < 'a' || pc > 'p')
				return BAD_ROOT_DEV;
			b += ((pc-'a') << 4);
			p++;
			break;
		}else if(match && pc != c){
			match = false;
		}
		p++;
		bdn++;
	}

	/* if we didn't match a device name, start over */
	if(!match){
		b = 0;
		p = devname;
	}

	/* then we read an index number */
	while(*p >= '0' && *p <= '9'){
		n = (n*10) + (*p - '0');
		p++;
		match = true;
	}

	/* string ends in junk? */
	switch(*p) {
		case 0:
		case '\n':
		case '\r':
			break;
		case ' ':
			parse_args(p);
			break;
		default:
			return BAD_ROOT_DEV;
	}

	if(match)
		return (b + n);
	else
		return BAD_ROOT_DEV;
}

/* So its in discard and thrown not on stack */
static char bootline[64];

uint16_t get_root_dev(void)
{
	uint16_t rd = BAD_ROOT_DEV;

	if (cmdline && *cmdline){
		rd = bootdevice(cmdline);
                cmdline=NULL;                   /* ignore cmdline if get_root_dev() is called again */
        }

	while(rd == BAD_ROOT_DEV){
		kputs("bootdev: ");
		udata.u_base = (uint8_t *)bootline;
		udata.u_sysio = 1;
		udata.u_count = sizeof(bootline)-1;
		udata.u_euid = 0;		/* Always begin as superuser */

		cdread(TTYDEV, O_RDONLY);	/* read root filesystem name from tty */
		rd = bootdevice(bootline);
	}

	return rd;
}
#else

inline uint16_t get_root_dev(void)
{
	return BOOTDEVICE;
}
#endif

void fuzix_main(void)
{
	/* setup state */
	inint = false;
	udata.u_insys = true;

#ifdef PROGTOP		/* FIXME */
	ramtop = (uaddr_t)PROGTOP;
#endif

	tty_init();

	if (d_open(TTYDEV, 0) != 0)
		panic(PANIC_NOTTY);

	/* Sign on messages */
	kprintf(
			"FUZIX version %s\n"
			"Copyright (c) 1988-2002 by H.F.Bower, D.Braun, S.Nitschke, H.Peraza\n"
			"Copyright (c) 1997-2001 by Arcady Schekochikhin, Adriano C. R. da Cunha\n"
			"Copyright (c) 2013-2015 Will Sowerbutts <will@sowerbutts.com>\n"
			"Copyright (c) 2014-2015 Alan Cox <alan@etchedpixels.co.uk>\nDevboot\n",
			sysinfo.uname);

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
	__hard_ei();		/* Physical interrupts on */
	kputs("ok.\n");

	/* initialise hardware devices */
	device_init();

        while(true){
            old_progptr = progptr;
            old_argptr = argptr;
            /* Get a root device to try */
            root_dev = get_root_dev();
            /* Mount the root device */
            kprintf("Mounting root fs (root_dev=%d, r%c): ", root_dev, ro ? 'o' : 'w');
            if(fmount(root_dev, NULLINODE, ro) == 0)
                break;
            kputs("failed\n");
            /* reset potentially altered state before prompting the user for command line again */
            progptr = old_progptr;
            argptr = old_argptr;
            ro = 0;
        }

	root = i_open(root_dev, ROOTINODE);
	if (!root)
		panic(PANIC_NOROOT);

	kputs("OK\n");

	/* finish building argv */
	complete_init();

	udata.u_cwd = i_ref(root);
	udata.u_root = i_ref(root);
	rdtime32(&udata.u_time);
	exec_or_die();
}

