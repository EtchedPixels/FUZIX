/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* CPM emulation capable */
#define CONFIG_CPM_EMU
/* 16K reported page size */
#define CONFIG_PAGE_SIZE	16
/* We use flexible 16K banks with a fixed common */
#define CONFIG_BANK16FC
/* You can put 2MB in a PCW with 3rd party add in cards, 512K base */
#define MAX_MAPS 128

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

#define CONFIG_FDC765

#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX 3
#define CONFIG_TD_NUM	2
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO

#undef CONFIG_NET
#undef CONFIG_NET_NATIVE

#define TICKSPERSEC 300		/* FIXME: double check - Ticks per second */
#define PROGBASE    0x0000	/* memory base of program */
#define PROGLOAD    0x0100	/* load base of program */
#define PROGTOP     0xBE00  	/* Top of program, base of U_DATA stash */


#define CONFIG_DYNAMIC_SWAP
#define SWAPDEV	    (swap_dev)
extern uint16_t swap_dev;
#define SWAP_SIZE   0x60 	/* 48K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vector and stash */
#define MAX_SWAPS   16
/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. We use the window at 0x4000 at all times.
 */
#define swap_map(x)	((uint8_t *)((((x) & 0x3FFF)) + 0x4000))

#define BOOT_TTY	(512 + 1)

#define CMDLINE		NULL

/* Device parameters */
#define NUM_DEV_TTY 3
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define CONFIG_LARGE_IO_DIRECT(x)	1


#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd,fd#"
