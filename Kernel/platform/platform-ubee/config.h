/* We have an RTC - well maybe (its optional) */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL	1	/* fast access */
/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Banked memory set up */
#define CONFIG_BANK_FIXED
#define MAX_MAPS	16		/* 512 KByte... */
#define MAP_SIZE	0x8000

/* Set these two for networking - no point right now */
/* #define CONFIG_NET */
/* #define CONFIG_NET_NATIVE */
/* Set this for IDE */
/* #define CONFIG_IDE */

#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_DYNAMIC_SWAP
#define CONFIG_LARGE_IO_DIRECT(x)	1

#define MAX_BLKDEV	4

#define CONFIG_BANKS	2	/* 2 x 32K */

/* For now we don't support resizing */
#define VT_WIDTH	80
#define VT_HEIGHT	25
#define VT_RIGHT	79
#define VT_BOTTOM	24

#define TICKSPERSEC 10	    /* Ticks per second */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0x7E00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   32 	    /* Memory needed per process */

#define SWAPDEV     (swap_dev)
#define SWAP_SIZE   0x40 	/* 32K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* vectors so its a round number of sectors */

#define MAX_SWAPS	16	/* Should be plenty */

#define swap_map(x)	((uint8_t *)(x))

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

extern uint16_t swap_dev;

#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd#"
