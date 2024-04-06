/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI

/* Select a banked memory set up */
#define CONFIG_BANK_FIXED
#define MAX_MAPS	2		/* 128K */
#define MAP_SIZE	0x8000
#define CONFIG_BANKS	2	/* 2 x 32K */

/*
 *	Define the program loading area (needs to match kernel.def)
 */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0x7E00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   32 	    /* Memory needed per process including stash */

#define CONFIG_SWAP
#define CONFIG_DYNAMIC_SWAP
#define SWAPDEV    (swap_dev)
extern uint16_t swap_dev;

#define SWAP_SIZE   0x40 	/* 32K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* vectors so its a round number of sectors */

#define MAX_SWAPS   16

#define swap_map(x)	((uint8_t *)(x))

#define CONFIG_TD_NUM		2
/* RC2014 style CF IDE */
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_8BIT
#define CONFIG_TINYIDE_INDIRECT
#define IDE_IS_8BIT(x)	1

#define BOOTDEVICENAMES "hd#"

/* We will resize the buffers available after boot. This is the normal setting */
#define CONFIG_DYNAMIC_BUFPOOL

#define CONFIG_LARGE_IO_DIRECT(x)	1

#define TICKSPERSEC	60	    /* Ticks per second */

#define CONFIG_VT
#define VT_WIDTH	64
#define VT_HEIGHT	48
#define VT_RIGHT	63
#define VT_BOTTOM	47
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL
#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX 3

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
#define CMDLINE	NULL	  /* Location of root dev name */
#define NUM_DEV_TTY 1	  /* How many tty devices does the platform support */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers. Must be 4+ and must match
                             kernel.def */
#define NMOUNTS	 3	  /* Number of mounts at a time */
#define TTY_INIT_BAUD B115200	/* Actually fixed */

#define plt_copyright()
