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
#define CONFIG_LARGE_IO_DIRECT
/* 32 x 16K pages, 3 pages for kernel, whatever the RAM disk uses */
#define MAX_MAPS	(32 - 3)

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 10      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xD000  /* Top of program, base of U_DATA copy */
/* FIXME: check this... for discard looks wrong */
#define KERNTOP     0xC000  /* Top of kernel (first 3 banks), base of shared bank */
#define PROC_SIZE   64	  /* Memory needed per process */

//#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern unsigned int swap_dev;
#define SWAP_SIZE   0x69 	/* 60.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xD200	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
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

#define MAX_BLKDEV 5	    /* 1 floppy, 4 IDE */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_NO_CLOCK

/* Floppy support */
#define CONFIG_FLOPPY		/* #define CONFIG_FLOPPY to enable floppy */
/* IDE/CF support */
#define CONFIG_IDE

#undef CONFIG_VFD_TERM         /* #define CONFIG_VFD_TERM to show console output on VFD display */

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	0	/* No keyboard to grab */

/* Core Networking support */
#define CONFIG_NET
/* User mode uIP TCP/IP daemon */
#define CONFIG_NET_NATIVE

#define NUM_DEV_TTY 2


/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

/* The Scott Baker SIO has a non-standard layout (it predates the official one) */
/* You'll need to define this if you have a Scott Baker SIO2 card, or submit
   a fancier autodetect! Also you'll need to change rc2014.s */
#undef CONFIG_SIO_BAKER

#define platform_copyright()		// for now