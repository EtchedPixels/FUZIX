/* System level configuration */

/* Set this if you have the RC2014 CF adapter at 0x10/0x90 */
#define CONFIG_RC2014_CF
/* Set this to be able to do networking over the second serial port */
#define CONFIG_RC2014_NET
/* Set this if you have the 8255 IDE adapter (mutually exclusive of RC2014_CF) */
#undef CONFIG_RC2014_PPIDE
/* Set this if you have the floppy interface */
#define CONFIG_RC2014_FLOPPY
/* Set this if you have a VFD interface */
#undef CONFIG_RC2014_VFD


#define OFTSIZE		64
#define ITABSIZE	48
#define PTABSIZE	24

/*
 *	Turn selections into system level defines
 */

#ifdef CONFIG_RC2014_CF
#define CONFIG_IDE
#endif
#ifdef CONFIG_RC2014_PPIDE
#define CONFIG_IDE
#define CONFIG_PPIDE
#endif
#ifdef CONFIG_RC2014_NET
/* Core Networking support */
#define CONFIG_NET
/* User mode uIP TCP/IP daemon */
#define CONFIG_NET_NATIVE
#endif
#ifdef CONFIG_RC2014_FLOPPY
#define CONFIG_FLOPPY
#endif
#ifdef CONFIG_RC2014_VFD
#define CONFIG_VFD_TERM
#endif

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* 32 x 16K pages, 3 pages for kernel, whatever the RAM disk uses */
#define MAX_MAPS	(32 - 3)

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 10      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */

//#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern unsigned int swap_dev;
#define SWAP_SIZE   0x69 	/* 60.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF200	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
//#define CONFIG_DYNAMIC_SWAP
/* Kept in bank 2 */
#define CONFIG_DYNAMIC_BUFPOOL
/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. For a simple banked setup there is no conversion
 *	needed so identity map it.
 */
#define swap_map(x)	((uint8_t *)((((x) & 0x3FFF)) + 0x4000))

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd,,rd"

#define NBUFS    5        /* Number of block buffers - must match kernel.def */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 5	    /* 1 floppy, 4 IDE */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_NO_CLOCK

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	0	/* No keyboard to grab */

#define NUM_DEV_TTY 4

/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define Z180_IO_BASE	0x40

#define platform_copyright()		// for now
