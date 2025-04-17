#define CONFIG_TD_NUM		1
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_INDIRECT
#define IDE_IS_8BIT(x)	0

#define CONFIG_LARGE_IO_DIRECT(x)	1  /* We support direct to user I/O */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
#define CONFIG_SWAP_ONLY
/* CP/M emulation */
#undef CONFIG_CPM_EMU

/* Required for Z80 single process mode (and faster) */
#define CONFIG_PARENT_FIRST

/* Settings for swap only mode */
#define CONFIG_SPLIT_UDATA
#define UDATA_SIZE 0x200
#define UDATA_BLKS 1

/* Input layer support */
#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX	3
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Keyboard contains non-ascii symbols */
#define CONFIG_UNIKEY
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL

#define CONFIG_BLKBUF_EXTERNAL

/* Custom banking */

/* We have one mapping from our working of memory */
#define MAX_MAPS	1
#define MAP_SIZE	0x8000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x8000U  /* also data base */
#define PROGLOAD    0x8000U  /* also data base */
#define PROGTOP	    0x10000UL  /* Top of program */
#define PROC_SIZE   32	  /* Memory needed per process */
#define MAXTICKS    10	  /* As our task switch is so expensive */

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    7       /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define MAX_BLKDEV 2	    /* 2 IDE drives, 2 SD drive */

#define SWAPBASE 0x8000
#define SWAPTOP  0x1000UL	/* FE00+ is udata, stacks etc */
#define SWAP_SIZE 0x41		/* 0x40 for image and 1 for udata */
/* We need to set the swaps up dynamically. In theory the counts are
   14 for DivIDE plus (512K RAM, of which 64K is kernel banks), and
   13 for ZXCF+ 512K (512K RAM 64K kernel banks 32K ResiDOS) but there
   is a 1MB version that would need 29.. which is silly. Set it to 16
   which is our process count and worst case */
/* FIXME: set max procs properly by swap space */
#define MAX_SWAPS	16
#define SWAPDEV  0x800	  /* Device for swapping (ram special). */

/* All our swap is to direct mapped space */
#define swap_map(x)		((uint8_t *)(x))

#define BOOTDEVICENAMES "hd#"

#define CONFIG_SMALL
