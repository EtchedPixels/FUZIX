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
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT
/* 32 x 16K pages, 3 pages for kernel, 16 pages for RAM disk */
#define MAX_MAPS	13

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 15      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   64	  /* Memory needed per process */

/* WRS: this is probably wrong -- we want to swap the full 64K minus the common code */
/* For now let's just use something and fix this up later when we have a swap device */
#define SWAP_SIZE   0x7F 	/* 63.5K in blocks (which is the wrong number) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xFF00	/* can we stop at the top? not sure how. let's stop short. */
#define MAX_SWAPS	10	    /* Well, that depends really, hmmmmmm. Pick a number, any number. */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "fd,rd"

//#define SWAPDEV  (256 + 1)  /* Device for swapping */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4	    /* 1 ROM disk, 1 RAM disk, 1 floppy, 1 SD card */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Floppy support */
#define CONFIG_FLOPPY		/* # define CONFIG_FLOPPY to enable floppy */

/* Optional ParPortProp board connected to PPI */
//#define CONFIG_PPP		/* #define CONFIG_PPP to enable as tty3 */

/* Device parameters */
#define NUM_DEV_RD 1
#define DEV_RD_PAGES 16		/* size of the RAM disk in pages */
#define DEV_RD_START 48		/* first page used by the RAM disk */

#ifdef CONFIG_PPP
	/* SD card in ParPortProp */
	#define DEVICE_SD
        #define SD_DRIVE_COUNT 1
	#define NUM_DEV_TTY 2

	/* ParPortProp as the console */
	#define BOOT_TTY (512 + 2)
#else
	#define NUM_DEV_TTY 1

	/* UART0 as the console */
	#define BOOT_TTY (512 + 1)
#endif

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
