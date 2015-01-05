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
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 8 60K banks, 1 is kernel */
#define MAX_MAPS	8
#define MAP_SIZE	0xF000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 100U   /* Ticks per second */
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
#define CMDLINE	(0x0081)  /* Location of root dev name */

//#define SWAPDEV  (256 + 1)  /* Device for swapping. (z80pack drive J) */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Hardware parameters */
#define Z180_IO_BASE       0x40
#define MARK4_IO_BASE      0x80

#define MAX_BLKDEV 3	    /* 2 IDE drives, 1 SD drive */

/* On-board IDE on Mark IV */
#define DEVICE_IDE
#define IDE_REG_BASE       MARK4_IO_BASE
#define IDE_8BIT_ONLY
#define IDE_REG_CS1_FIRST

/* On-board SD on Mark IV */
#define DEVICE_SD
#define SD_DRIVE_COUNT 1

/* On-board DS1302 on Mark IV, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Optional PropIOv2 board on ECB bus */
//#define CONFIG_PROPIO2		/* #define CONFIG_PROPIO2 to enable as tty3 */
#define PROPIO2_IO_BASE		0xA8

/* Device parameters */
#ifdef CONFIG_PROPIO2
	#define NUM_DEV_TTY 3

	/* PropIO as the console */
	#define BOOT_TTY (512 + 3)
#else
	#define NUM_DEV_TTY 2

	/* ASCI0 as the console */
	#define BOOT_TTY (512 + 1)
#endif

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
