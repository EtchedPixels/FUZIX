/* System level configuration */

/* Set this if you have the RC2014 CF adapter at 0x10/0x90 */
#define CONFIG_RC2014_CF
/* Set this if you have the 8255 IDE adapter (mutually exclusive of RC2014_CF) */
#undef CONFIG_RC2014_PPIDE
/* Set this if you have the floppy interface */
#undef CONFIG_RC2014_FLOPPY

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
#ifdef CONFIG_RC2014_FLOPPY
#define CONFIG_FLOPPY
#endif

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace unless
   they came from floppy disk */
#define CONFIG_LARGE_IO_DIRECT(x)	((x) != 1)
/* 32 x 16K pages, 3 pages for kernel, whatever the RAM disk uses */
#define MAX_MAPS	(32 - 3)

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 50      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xEE00  /* Top of program, base of U_DATA copy */
#define KERNTOP     0xC000  /* Top of kernel (first 3 banks), base of shared bank */

/* Adjust copy_common if you touch the above */

//#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x78 	/* Program + udata in blocks */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF000	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
//#define CONFIG_DYNAMIC_SWAP
/* We have lots of RAM so make better use of it for disk buffers. We grab
   a 16K page and use it as our disk cache */
#define CONFIG_BLKBUF_EXTERNAL
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

#define NBUFS    32       /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 2	    /* 2 IDE */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
/* The DS1302 reading is painfully slow - resync only every 20 seconds */
#define CONFIG_RTC_INTERVAL	200
#define CONFIG_RTC_FULL

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	0	/* No keyboard to grab */

#define NUM_DEV_TTY 2

/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define plt_copyright()		// for now
