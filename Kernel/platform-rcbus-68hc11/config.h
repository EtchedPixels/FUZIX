/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Acct syscall support */
#undef CONFIG_ACCT
/* Multiple processes in memory at once */
#define CONFIG_MULTI

/*
 *	Currently we are using the 512K/512K RAM card. We could ue the 8085
 *	style MMU card and linear RAM and that might be better
 */
/* 16K reported page size */
#define CONFIG_PAGE_SIZE	48
/* We use flexible 16K banks with a fixed common */
#define CONFIG_BANK16
#define CONFIG_BANKS	4

#define MAX_MAPS	(32-3)

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Arguments are tricky. The 680x binaries stack one way the 68HC11 the other.
   We deal with that in the syscall stubs and in crt0 */
#define CONFIG_CALL_R2L

#define TICKSPERSEC 20	    /* Ticks per second */

#define MAPBASE	    0x0000  /* We map from 0 */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xEE00

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
/* We swap a range including the internal I/O window. This is safe because we
   map the memory into the swap window not directly */
#define SWAP_SIZE   0x7A 	/* 61K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF400	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. We use the second 16K window.
 */
#define swap_map(x)	((uint8_t *)((((x) & 0x3FFF)) + 0x4000))

#define CONFIG_IDE
#define MAX_BLKDEV 3
#define CONFIG_SD
#define SD_DRIVE_COUNT 1

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_EXTENDED
#define CONFIG_RTC_INTERVAL	100

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"

/* 68HC11 specific stuff */
#define IOBASE	0xF000

#define DP_BASE 0x0000
#define DP_SIZE 0x00C0	/* C0-FF is for the kernel */
