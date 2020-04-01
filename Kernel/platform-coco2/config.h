/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Pure swap */
#define CONFIG_SWAP_ONLY
#define CONFIG_SPLIT_UDATA
#define UDATA_BLKS 1
#define CONFIG_USERMEM_DIRECT

#define CONFIG_LEVEL_0		/* Minimal system */

#define CONFIG_BANKS	1
/* And swapping */
#define SWAPDEV     0x0		/* Uses part of IDE slice 0 */
#define SWAP_SIZE   0x39	/* 512 byte blocks */
#define SWAPBASE    0x8000	/* We swap the lot, including stashed uarea */
#define SWAPTOP     0xF200	/* so it's a round number of 512 byte sectors */
#define UDATA_SIZE  0x0200	/* one block */
#define MAX_SWAPS   32

#define PTABSIZE	8

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_VT_SIMPLE

extern unsigned char vt_mangle_6847(unsigned char c);
#define VT_MAP_CHAR(x)	vt_mangle_6847(x)

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23
#define VT_INITIAL_LINE	0

#define VT_BASE	((uint8_t *)0x0200)

/* RS/Tandy Color Computer keyboard */
#undef CONFIG_COCO_KBD

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x8000  /* also data base */
#define PROGLOAD    0x8000  /* also data base */
#define PROGTOP     0xF200  /* Top of program */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   513	 /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define swap_map(x)	((uint8_t *)(x))

#define CONFIG_IDE

#define platform_copyright(x)

