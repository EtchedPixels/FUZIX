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
/* CP/M emulation */
#define CONFIG_CPM_EMU
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Multiple VT support */
#define CONFIG_VT_MULTI
#define MAX_VT	2
/* We need a 6x8 font to upload to the vdp */
#define CONFIG_FONT6X8
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 10 48K banks, 1 is kernel */
#define MAX_MAPS	10
#define MAP_SIZE	0xC000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 50      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xBD00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   48	    /* Memory needed per process */

/* Undefine this section if you don't have a silicon disc for swap. It's not
   really important - you can get 10 processes on a 512K unit anyway */
#define SWAP_SIZE   0x60 	/* 48K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vectors so its a round number of sectors */
#define MAX_SWAPS   64		/* How many swaps per disc */
#define SWAPDEV  ((8*256) + 0)  /* Device for swapping. - first silicon disk */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 4	/* Will be 4 two monitors, two serial */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define NBUFS    16       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Terminal definitions */
#define VT_WIDTH	vt_twidth[curtty]
#define VT_HEIGHT	24
#define VT_RIGHT	vt_tright[curtty]
#define VT_BOTTOM	23

#define swap_map(x)	((uint8_t *)(x))

#define platform_discard()
