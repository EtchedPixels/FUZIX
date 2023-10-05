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
 *	Lots to figure out here yet
 */

/* 8K reported page size */
#define CONFIG_PAGE_SIZE	8
/* We use flexible 8K banks */
#define CONFIG_BANK8
#define CONFIG_BANKS	8
#define MAX_MAPS	128
#define PAGE_INVALID	0xFF		/* An alias mapping of the EPROM */
#define TOP_SIZE	0x1000

/* Permit large I/O requests to bypass cache and go direct to userspace */
/* FIXME: get it working first - the memory/disk interactions are weird */
#define CONFIG_LARGE_IO_DIRECT(x)	0

#define TICKSPERSEC 20	    /* Ticks per second */

#define MAPBASE	    0x0000  /* We map from 0 */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xF000

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x60 	/* 48K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xC000	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define swap_map(x)	((uint8_t *)(x))

#define CONFIG_FONT6X8
#define CONFIG_VT
#define CONFIG_VT_MULTI

#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX	3

/* Vt definitions */
#define VT_WIDTH	80
#define VT_HEIGHT	24
#define VT_RIGHT	79
#define VT_BOTTOM	23

/* MODE TEXT2 supports up to 16 VT's */
#define MAX_VT          4

#define MAX_BLKDEV 2

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
