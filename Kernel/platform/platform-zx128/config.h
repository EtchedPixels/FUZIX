#define CONFIG_LEVEL_0

#define CONFIG_IDE
//#define CONFIG_BETADISK

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

/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Keyboard contains not ascii symbols */
#define CONFIG_UNIKEY
/* We don't need a font: We will use the ROM font */
#undef CONFIG_FONT8X8
#undef CONFIG_FONT8X8SMALL

/* Custom banking */

/* We have two mappings from our 128K of memory */
#define MAX_MAPS	2
#define MAP_SIZE	0x8000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x8000  /* also data base */
#define PROGLOAD    0x8000  /* also data base */
#define PROGTOP     0xFD00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   32	  /* Memory needed per process */

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
/* #define SWAPDEV  2051 */ /* Microdrive 3 : FIXME - configure and probe */
#define NBUFS    9       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
#define MAX_BLKDEV 2	    /* 2 IDE drives, 1 SD drive */

#define SWAPBASE 0x8000
#define SWAPTOP  0x10000UL
#define SWAP_SIZE 0x40
#define MAX_SWAPS 3		/* For now */

/* All our pages get mapped into the top 16K bank for swapping use */
#define swap_map(x)		((uint8_t *)(x|0xC000))

#define plt_discard()

/* Betadisk functions do not work with modern procedures */
#ifdef CONFIG_BETADISK
#define CONFIG_LEGACY_EXEC
#endif
