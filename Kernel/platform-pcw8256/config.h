/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* CPM emulation capable */
#define CONFIG_CPM_EMU
/* 16K reported page size */
#define CONFIG_PAGE_SIZE	16
/* We use flexible 16K banks so use the helper */
#define CONFIG_BANK16
#define MAX_MAPS 16
#define MAX_SWAPS 16

#define CONFIG_BANKS	4	/* 4 banks 16K page size */

/* VT layer required */
#define CONFIG_VT
/* Has soem keys in the unicode range */
#define CONFIG_UNIKEY
/* We want the 8x8 font */
#define CONFIG_FONT8X8
/* Vt definitions */
#define VT_WIDTH	90
#define VT_HEIGHT	32
#define VT_RIGHT	89
#define VT_BOTTOM	31

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x0000	/* memory base of program */
#define PROGLOAD    0x0100	/* load base of program */
#define PROGTOP     0xF000  	/* Top of program, base of U_DATA */

#define SWAP_SIZE   0x80 	/* 64K in blocks (we actually don't need all of it FIXME) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xF400	/* vectors. We have to swap 256 bytes of
                                   common as well */

#define BOOT_TTY	(512 + 1)

#define CMDLINE		NULL

/* Device parameters */
#define NUM_DEV_TTY 3
#define NDEVS    16       /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  5	  /* Device for swapping. */
#define NBUFS    6        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */



