#define CONFIG_LARGE_IO_DIRECT(x)	1  /* We support direct to user I/O */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* CP/M emulation */
#undef CONFIG_CPM_EMU

/* Select a banked memory set up */
#define CONFIG_BANK_FIXED
/* This is the number of banks of user memory available (maximum) */
#define MAX_MAPS	2
/* We have 0000-1FFF common */
#define MAP_SIZE	0xC000
/* How many banks do we have in our address space */
#define CONFIG_BANKS	1	/* 1 x 56K */

/* Input layer support */
#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX	3
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Keyboard contains non-ascii symbols */
#define CONFIG_UNIKEY
#define CONFIG_FONT8X8

#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_DYNAMIC_SWAP

/* Custom banking */

/* Vt definitions */
/* TODO: 80 column */
#define VT_WIDTH	64
#define VT_HEIGHT	24
#define VT_RIGHT	63
#define VT_BOTTOM	23

/* We can't start at 0x2000 due to brokenness in the Uno implementation */
#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x4000  /* also data base */
#define PROGLOAD    0x4000  /* also data base */
#define PROGTOP     0xFE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   48	  /* Memory needed per process */
#define MAXTICKS    10	  /* As our task switch is so expensive */

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
#define MAX_BLKDEV 2	    /* 2 IDE drives, 2 SD drive */

#define SWAPBASE 0x4000
#define SWAPTOP  0x10000UL
#define SWAP_SIZE 0x60
#define MAX_SWAPS	16
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */

#define CONFIG_TD
#define CONFIG_TD_NUM	2
/* SD support */
#define TD_SD_NUM 1
#define CONFIG_TD_SD
/* Emulator for this platform needs bug workarounds */
#define CONFIG_TD_SD_EMUBUG
#define SD_SPI_CALLTYPE __z88dk_fastcall

/* We need to direct map things because of the Timex MMU modes */
#define swap_map(x)		((uint8_t *)x)

#define BOOTDEVICENAMES "hd#"

#define CONFIG_SMALL
