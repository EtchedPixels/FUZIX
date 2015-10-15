./Applications/util/tiddles.c:  /* FIXME: needs buffer algorithms */
./Applications/util/tiddles.c:  /* FIXME: count this into spew rebuild hints ? */
./Applications/util/tiddles.c:/* Load the input file: FIXME: what to do about overlong lines */
./Applications/util/tiddles.c:  /* FIXME: overflow check ? */
./Applications/util/tiddles.c:        /* line too long ?? truncate or wrap ?? FIXME */
./Applications/util/tiddles.c:/* FIXME: end line without \n ?? */
./Applications/util/tiddles.c:/* FIXME: need to blank lines below the end of file ! */
./Applications/util/tiddles.c:  /* FIXME: play with this - probably better to look ahead if line right
./Applications/util/tiddles.c:  if (cursory >= bottom_row)	/* FIXME need to consider if lines below 
./Applications/util/chmod.c:#define PATH_MAX	512	/* FIXME: it's a sysconf shouldn't be
./Applications/util/mkdir.c:    /* FIXME: Size check ! */
./Applications/util/ll.c:/* FIXME: use readdir */
./Applications/util/ll.c:    /* FIXME: use readdir etc */
./Applications/util/fdisk.c:void list_types(void)	/* FIXME - Should make this more flexible */
./Applications/util/fdisk.c:void set_type(void)  /* FIXME - Should make this more flexible */
./Applications/util/fdisk.c:	partition=ptbl+i;   /* FIXME /--- gotta be a better way to do this ---\ */
./Applications/util/fdisk.c:		strncpy(dev,argv[i],256); /* FIXME - Should be some value from a header */
./Applications/util/mkfs.c:    uint8_t       s_timeh;	/* bits 32-40: FIXME - wire up */
./Applications/util/ln.c:#if defined(S_ISLNK) && 0	/* FIXME */
./Applications/util/fdisk.h:char dev[256]; /* FIXME - should be a #define'd number from header file */
./Applications/util/fsck.c:	/* FIXME: named pipes ? */
./Applications/util/sort.c:#define OPEN_MAX	8		/* HACK FIXME */
./Applications/util/rm.c:/* FIXME: need -r -v -i etc */
./Applications/ld09/ld.c:#if 0	/* FIXME */
./Applications/levee/exec.c://FIXME!!		system(execstr);
./Applications/V7/cmd/test.c:	/* FIXME: should use stat and parse permissions */
./Applications/V7/cmd/ac.c:/* FIXME: do modern style TZ */
./Applications/V7/cmd/cron.c:/* FIXME: hardcoded UID 1 assumption */
./Applications/V7/cmd/atrun.c:	/* FIXME: cheaper to do the copy inline */
./Applications/V7/cmd/su.c:/* FIXME: su - and other semantics of later su versions */
./Applications/V7/cmd/wall.c:	/* FIXME: needs to use unix I/O and O_NDELAY, plus set an alarm */
./Applications/V7/cmd/sh/xec.c:				argn = getarg((void *)t);/*FIXME*/
./Applications/V7/cmd/sh/xec.c:						else if ((a1 == 0 && (a1 = (char *)homenod.namval) == 0) || chdir(a1) < 0) /* FIXME */
./Applications/V7/cmd/sh/xec.c:							execexp(a1, (UFD)&com[2]);	/* FIXME */
./Applications/V7/cmd/sh/msg.c:/* FIXME: align with actual OS! */
./Applications/V7/cmd/sh/macro.c:	push((void *)&fb);/*FIXME*/
./Applications/V7/cmd/sh/service.c:/* FIXME: errno from header */
./Applications/V7/cmd/sh/service.c:/* FIXME: put into a header */
./Applications/V7/cmd/sh/service.c:extern BOOL nosubst;	/* FIXME */
./Applications/V7/cmd/sh/name.c:		replace((char **)&n->namval, v);	/* FIXME: check safe */
./Applications/V7/cmd/sh/cmd.c:		t = makefork(0, /*FIXME*/(void *)p);
./Applications/V7/cmd/sh/fault.c:	/* FIXME: Was a test of the low bit.. not clear this is the correct translation of V7 internals */
./Applications/V7/cmd/sh/fault.c:	/* Again was a zero test for ignsig, unclear if correct translation FIXME */
./Applications/V7/cmd/sh/io.c:	f->fend = length(s) + (f->fnxt = (char *)s);/*FIXME review */
./Applications/V7/cmd/sh/print.c:/* FIXME: use libc */
./Applications/V7/cmd/sh/print.c:/* FIXME: use libc */
./Applications/V7/cmd/sh/glob.c:/* FIXME */
./Applications/V7/cmd/sh/glob.c:/* FIXME */
./Applications/V7/cmd/diffh.c:	/* FIXME: buffer length is not sanely checked, should
./Applications/V7/cmd/crypt.c:/* FIXME: this is *not* strong crypto */
./Standalone/chmem.c:  /* FIXME : add 6809 but remember its big endian! */
./Standalone/xfs1b.c:	/* FIXME: endiam swapping here */
./Standalone/fsck.c:        /* FIXME: named pipes.. */
./Library/include/time.h:#define CLOCKS_PER_SEC	100		/* FIXME: sysconf */
./Library/include/termios.h:#define VEOL		1	/* partial - FIXME, EOF in input */
./Library/include/proc.h:    uint16_t	p_top;		/* Copy of u_top : FIXME: usize_t */
./Library/include/setjmp.h:	/* FIXME: need to add alt registers */
./Library/libs/strchr.c:/* FIXME: asm version ?? */    
./Library/libs/sleep.c:   FIXME: probably worth having a Z80 asm version of this */
./Library/libs/pathconf.c:      return 4096;		/* FIXME: wrong but need to sort socket
./Library/libs/clock.c:/* FIXME: CLOCKS_PER_SEC query */
./Library/libs/setlocale.c:/* FIXME: add nl_langinfo() */
./Library/libs/readdir.c:	buf->d_off = -1;	/* FIXME */
./Library/libs/curses/doupdt.c:   FIXME: try and move away from sucking in stdio in curses and in
./Library/libs/curses/prntscan.c:   remove the length limit and buffer but that's a FIXME */
./Library/libs/stat.c:  /* FIXME: these 3 will change when the kernel API is fixed to pass
./Library/libs/sysconf.c:      /* FIXME: read the regexp code */
./Library/libs/setjmp.c: * FIXME: Add alt register saves
./Library/libs/gettimeofday.c:    /* FIXME: although this is obsolete */
./Library/libs/free.c:                /* FIXME: void * cast appears to be a cc65 bug */
./Library/libs/termcap.c: * FIXME:
./Library/libs/isatty.c:	/* FIXME: should do a tty ioctl */
./Library/libs/execl.c:/* FIXME: The 6502 calling sequence means these need a different implementation */
./Kernel/platform-8086test/config.h:#define SWAPDEV 6	/* FIXME */
./Kernel/platform-8086test/config.h:/* FIXME */
./Kernel/platform-8086test/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/include/graphics.h:/* FIXME: need a way to describe/set modes if multiple supported */
./Kernel/include/tty.h:#define VEOL		1	/* partial - FIXME, EOF in input */
./Kernel/include/kernel.h:/* FIXME: if we could split the data and the header we could keep blocks
./Kernel/include/kernel.h:    uint8_t       s_timeh;	/* bits 32-40: FIXME - wire up */
./Kernel/include/kernel.h:/* FIXME: finish signal handling */
./Kernel/include/kernel.h:extern void pagemap_add(uint8_t page);	/* FIXME: may need a page type for big boxes */
./Kernel/platform-dragon/devtty.c:		/* FIXME: do proper mode setting */
./Kernel/platform-dragon/devfd.c:        /* FIXME: should we try the other half and then bale out ? */
./Kernel/platform-dragon/config.h:#define SWAPDEV 6	/* FIXME */
./Kernel/platform-dragon/config.h:/* FIXME */
./Kernel/platform-dragon/config.h:/* FIXME: This will move once we put the display in the kernel bank and
./Kernel/platform-dragon/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/dev/z80pack/devlpr.c:        /* FIXME: tidy up ugetc and sysio checks globally */
./Kernel/dev/z80pack/devfd.c:    /* Read the disk in four sector chunks. FIXME We ought to cache the geometry
./Kernel/dev/blkdev.c:/* FIXME: this would tidier and handle odd partition types sanely if split
./Kernel/dev/devdw.c:        /* FIXME: should we try the other half and then bail out ? */
./Kernel/dev/devdw.c:/* FIXME: for bit-banger transport (not Becker) we should set up
./Kernel/devio.c:	/* FIXME: this can generate a lot of d_flush calls when you have
./Kernel/devio.c:/* FIXME: To do banking without true 'far' pointers we need to figure
./Kernel/devio.c:	/* Not as clean as would be ideal : FIXME */
./Kernel/cpu-68hc11/cpu.h:   non-reentrant functions static - FIXME, evaluate this carefully esp
./Kernel/cpu-68hc11/cpu.h:   uint32_t high;	   /* FIXME: check this matches long long */
./Kernel/simple.c: *	- 16bit address space (FIXME: should be made 32bit clean)
./Kernel/platform-coco3/devtty.c:/* FIXME: these should use memmove */
./Kernel/platform-coco3/config.h:/* FIXME */
./Kernel/platform-coco3/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/flat.c://FIXME	mem_switch(proc);
./Kernel/usermem.c:	/* FIXME: for Z80 we should make this a udata field so that cp/m
./Kernel/platform-zeta-v2/devtty.c:		/* FIXME: implement */
./Kernel/platform-6502test/config.h:#define SWAPDEV 6	/* FIXME */
./Kernel/platform-6502test/config.h:/* FIXME */
./Kernel/platform-6502test/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-6502test/devrd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/platform-trs80/devlpr.c:        /* FIXME: tidy up ugetc and sysio checks globally */
./Kernel/platform-trs80/devgfx.c: *	FIXME: GETPIXEL, direct raw I/O access, scrolling, rects and
./Kernel/platform-trs80/devfd.c:        /* FIXME: We force DD for now */
./Kernel/platform-trs80/devfd.c:        /* FIXME: should we try the other half and then bale out ? */
./Kernel/platform-trs80/devhd.c:		/* FIXME: should we try the other half and then bale out ? */
./Kernel/syscall_exec16.c:			/* FIXME: allow for async queued I/O here. We want
./Kernel/syscall_exec16.c:	/* FIXME: In the execve case we may on some platforms have space
./Kernel/platform-pcw8256/devlpr.c:        /* FIXME: tidy up ugetc and sysio checks globally */
./Kernel/platform-pcw8256/devtty.c:	{'.', '/', ';', '$'/*FIXME*/, 'p', '[', '-', '='},
./Kernel/platform-pcw8256/devtty.c:	{KEY_DEL, '.', 13, KEY_F7, KEY_MINUS, KEY_CANCEL, 0, KEY_F5},	/* FIXME: js line */
./Kernel/platform-pcw8256/devtty.c:	{0, 0, 0, 0, 0, 0, 0, 0}	/* FIXME: js 2 */
./Kernel/platform-pcw8256/devtty.c:	{0, 0, 0, 0, 0, 0, 0, 0},	/* FIXME: js 1 */
./Kernel/platform-pcw8256/devtty.c:	{0, 0, 0, 0, 0, 0, 0, 0}	/* FIXME: js 2 */
./Kernel/platform-pcw8256/devtty.c:/* FIXME: keyboard repeat
./Kernel/platform-pcw8256/devfd.c://FIXME    while(!(fd_send(0x04, minor) & 0x20));
./Kernel/platform-pcw8256/devfd.c:        track[minor] = ntrack;//FIXME??fd765_statbuf[1] & 0x7F;
./Kernel/platform-pcw8256/config.h:#define SWAP_SIZE   0x80 	/* 64K in blocks (we actually don't need all of it FIXME) */
./Kernel/platform-pcw8256/pcw8256.h:__sfr __at 0xFC par9512;		/* FIXME: the 9512 are Z180 type decodes */
./Kernel/platform-6809test/config.h:#define SWAPDEV 6	/* FIXME */
./Kernel/platform-6809test/config.h:/* FIXME */
./Kernel/platform-6809test/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-6809test/devrd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/start.c:/* FIXME: pass remainder of boot argument to init */
./Kernel/start.c:		 /* FIXME: space trailing copy the rest into init args */
./Kernel/bank16k_low.c: *	FIXME: We also need to swap out page 0/1 on 6502 (ZP and S) as well
./Kernel/vt.c:	/* FIXME: need to address the multiple vt switching case
./Kernel/vt.c:/* FIXME: these should use memmove */
./Kernel/platform-tgl6502/config.h:#define SWAPDEV 	257	/* FIXME */
./Kernel/platform-tgl6502/config.h:/* FIXME: we need to swap the udata separately */
./Kernel/platform-68hc11test/config.h:#define SWAPDEV 6	/* FIXME */
./Kernel/platform-68hc11test/config.h:/* FIXME: udata swap separately */
./Kernel/platform-68hc11test/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-68hc11test/devrd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/platform-socz80/devrd.c:    /* FIXME Should be able to avoid the __critical once bank switching is fixed */
./Kernel/platform-socz80/ethernet.c:		/* FIXME: check link up */
./Kernel/platform-socz80/ethernet.c:	/* Set the mac to AAAAAAC0FFEE for now FIXME */
./Kernel/platform-mtx/devlpr.c:		/* Strobe - FIXME: should be 1uS */
./Kernel/platform-mtx/devsil.c: * FIXME: would be sensible to add swap support to this driver
./Kernel/platform-mtx/devtty.c:/* FIXME: this will eventually vary by tty so we'll need to either load
./Kernel/platform-mtx/devtty.c:/* FIXME: need to wrap vt_ioctl so we switch to the right tty before asking
./Kernel/platform-mtx/devfd.c:        /* FIXME: should we try the other half and then bale out ? */
./Kernel/process.c:			/* FIXME: core dump on some signals */
./Kernel/process.c:	/* FIXME:
./Kernel/process.c:	/* FIXME: send SIGCLD here */
./Kernel/process.c:	/* FIXME: POSIX.1 says that SIG_IGN for SIGCLD means don't go
./Kernel/syscall_fs2.c:	/* FIXME: needs updating once we pack top bits
./Kernel/syscall_fs2.c:			udata.u_error = ENFILE;	/* FIXME, should be set in newfile
./Kernel/syscall_fs2.c:	/* FIXME: ATIME ? */
./Kernel/swap.c:/* FIXME: clean this up by having a common i/o structure to avoid
./Kernel/platform-plus3/devfd.c:            fdc_seek();		/* FIXME: error handling */
./Kernel/platform-plus3/devfd.c:            /* FIXME: need to return status properly and mask it */
./Kernel/platform-plus3/devfd.c:        if (tries > 1)		/* FIXME: set drive */
./Kernel/platform-plus3/main.c://FIXME floppy_timer();
./Kernel/syscall_proc.c:	   FIXME: if we get more complex mapping rule types then we may
./Kernel/syscall_proc.c:	/* FIXME: move this scan into the main loop and also error
./Kernel/malloc.c:	/* FIXME: check cast */
./Kernel/malloc.c:/* FIXME: We ought to keep this as a running total */
./Kernel/bankfixed.c: *	FIXME: we can write out base - p_top, then the udata providing
./Kernel/platform-ubee/devlpr.c:        /* FIXME: tidy up ugetc and sysio checks globally */
./Kernel/platform-ubee/devfd.c:        /* FIXME: We force DD for now. Side isn't handled here */
./Kernel/platform-ubee/devfd.c:    /* Double sided assumed FIXME */
./Kernel/platform-ubee/devhd.c:   FIXME */
./Kernel/platform-ubee/devhd.c:	For now just use double sided : FIXME */
./Kernel/inode.c:				/* FIXME: allow for async queued I/O here. We want
./Kernel/inode.c:		/* FIXME: this will hang if you ever write > 16 * BLKSIZE
./Kernel/inode.c:			/* FIXME: O_SYNC */
./Kernel/inode.c:	/* FIXME: for 32bit we will need to check for overflow of the
./Kernel/inode.c: *	FIXME: could we rewrite this so we just passed the oft slot and
./Kernel/inode.c: *	FIXME. Need so IS_TTY(dev) defines too and minor(x) etc
./Kernel/bank16k.c: *	FIXME: bank16k should only read/write out the banks that are in use
./Kernel/bank16k.c: * FIXME: bank16k should only read/write out the banks that are in use
./Kernel/platform-nc100/devtty.c:		/* FIXME: need to check uart itself to see wake cause */
./Kernel/platform-nc100/devtty.c:		/* FIXME: Power button */
./Kernel/platform-nc100/devtty.c:		/* FIXME: FDC interrupt */
./Kernel/platform-nc100/devaudio.c:/* FIXME? - worth making a pwait_irq or similar 'wait for an IRQ to clear
./Kernel/platform-zx128/devfd.c:    /* FIXME fd_map support
./Kernel/platform-zx128/devfd.c:        /* FIXME: need to do SD v DD detection */
./Kernel/platform-zx128/config.h:#define SWAPDEV  2051	/* Microdrive 3 : FIXME - configure and probe */
./Kernel/tty.c:        /* FIXME: racy - need to handle IRQ driven carrier events safely */
./Kernel/platform-atarist/devtty.c:		/* FIXME: do proper mode setting */
./Kernel/platform-atarist/devfd.c:        /* FIXME: should we try the other half and then bale out ? */
./Kernel/platform-atarist/config.h:#define CONFIG_BANKS	1		/* FIXME */
./Kernel/platform-atarist/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-px4plus/sio.c:  /* FIXME: save old modem bits, baud etc. We can't do this via hw
./Kernel/platform-px4plus/sio.c:  /* FIXME: disable ART IRQ */
./Kernel/platform-px4plus/sio.c:  /* FIXME */
./Kernel/platform-px4plus/sio.c: *	FIXME:
./Kernel/platform-px4plus/devfd.c:    /* FIXME */
./Kernel/platform-px4plus/config.h:/* FIXME: the OVL timer isn't quite 100/sec and we have an accurate 1Hz
./Kernel/platform-px4plus/config.h:/* FIXME: treat the banks of the ramdisc as memory not swap, also trim
./Kernel/platform-px4plus/devices.c:/* FIXME: find correct initial value */
./Kernel/devsys.c:    /* FIXME: this needs to be a per CPU value */
./Kernel/platform-msx2/devfd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/platform-msx2/devfd.c:        /* FIXME: Do stuff */
./Kernel/platform-msx2/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-msx2/devhd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/platform-msx2/devhd.c:        /* FIXME: Do stuff */
./Kernel/syscall_net.c:	/* We need an inode : FIXME - do we want a pipedev aka Unix ? */
./Kernel/syscall_net.c:		/* FIXME: return EINPROGRESS not EINTR for SS_CONNECTING */
./Kernel/syscall_net.c:/* FIXME: do we need the extra arg/flags or can we fake it in user */
./Kernel/tools/trslabel.c:  /* FIXME: should fill in matching LBA bits */
./Kernel/tools/bankld/lkelf.c:  ehdr.e_machine = EM_68HC08; /* FIXME: get rid of hardcoded value - EEP */
./Kernel/tools/binmunge.c:  /* FIXME: we could have per bank stubs in ROM and not waste precious
./Kernel/platform-dragon-nx32/devtty.c:		/* FIXME: do proper mode setting */
./Kernel/platform-dragon-nx32/devtty.c:/* FIXME: shouldn't COCO shiftmask also differ ??? 0x02 not 0x40 ?? */
./Kernel/platform-dragon-nx32/devfd.c:        /* FIXME: should we try the other half and then bale out ? */
./Kernel/platform-dragon-nx32/config.h:/* FIXME: This will move once we put the display in the kernel bank and
./Kernel/platform-dragon-nx32/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-msx1/devlpr.c:		/* FIXME: delay needed */
./Kernel/platform-msx1/devfd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/platform-msx1/devfd.c:        /* FIXME: Do stuff */
./Kernel/platform-msx1/config.h:#define TICKSPERSEC 50   /* Ticks per second (actually should be dynamic FIXME) */
./Kernel/platform-msx1/config.h:                            /* Temp FIXME set to serial port for debug ease */
./Kernel/platform-msx1/devhd.c:    /* FIXME: raw is broken unless nicely aligned */
./Kernel/platform-msx1/devhd.c:        /* FIXME: Do stuff */
