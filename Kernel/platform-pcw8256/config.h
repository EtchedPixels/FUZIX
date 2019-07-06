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
/* We use flexible 16K banks with a fixed common */
#define CONFIG_BANK16FC
#define MAX_MAPS 16
#define MAX_SWAPS 16

#define CONFIG_BANKS	4	/* 4 banks 16K page size */

/* VT layer required */
#define CONFIG_VT
/* Has some keys in the unicode range */
#define CONFIG_UNIKEY
/* We want the 8x8 font */
#define CONFIG_FONT8X8
/* Vt definitions */
#define VT_WIDTH	90
#define VT_HEIGHT	32
#define VT_RIGHT	89
#define VT_BOTTOM	31

#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX 2	/* We could in theory do full up/down but later */
#define MAX_BLKDEV	1	/* UIDE or FIDHD never both */
#define CONFIG_IDE	/* Has an IDE controller - maybe anyway: UIDE */
#define CONFIG_NET
#define CONFIG_NET_NATIVE

#define TICKSPERSEC 300		/* FIXME: double check - Ticks per second */
#define PROGBASE    0x0000	/* memory base of program */
#define PROGLOAD    0x0100	/* load base of program */
#define PROGTOP     0xBE00  	/* Top of program, base of U_DATA stash */

#define SWAP_SIZE   0x60 	/* 48K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vector and stash */

#define BOOT_TTY	(512 + 1)

#define CMDLINE		NULL

/* Device parameters */
#define NUM_DEV_TTY 3
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  5	  /* Device for swapping. */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define CONFIG_LARGE_IO_DIRECT(x)	1

#define swap_map(x)	(uint8_t *)(x)

#define platform_discard()
#define platform_copyright()

#define BOOTDEVICENAMES "hd,fd#"
