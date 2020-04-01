#define CONFIG_SD
#define SD_DRIVE_COUNT	2
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
#define MAX_MAPS	3		/* 512 KByte... minus the high one */
/* How big is each bank - in our case 32K, 48K is actually more common. This
   is hardware dependant */
#define MAP_SIZE	0xA000
/* How many banks do we have in our address space */
#define CONFIG_BANKS	1	/* 2 x 32K */

/* Input layer support */
#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX	3
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Keyboard contains non-ascii symbols */
#define CONFIG_UNIKEY
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL

#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_DYNAMIC_SWAP

/* Custom banking */

/* Vt definitions */
#define VT_WIDTH	64
#define VT_HEIGHT	24
#define VT_RIGHT	63
#define VT_BOTTOM	23

/* We can do better than this but we need to sort out the buffer banking first */
#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x6000  /* also data base */
#define PROGLOAD    0x6000  /* also data base */
#define PROGTOP     0xFE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   40	  /* Memory needed per process */
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

#define SWAPBASE 0x8000
#define SWAPTOP  0x10000UL
#define SWAP_SIZE 0x40
#define MAX_SWAPS	16
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */

/* We need to direct map things because of the Timex MMU modes */
#define swap_map(x)		((uint8_t *)x)

#define BOOTDEVICENAMES "hd#"
