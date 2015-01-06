/* Set if you want RTC support and have an RTC on ports 0xB0-0xBC */
#define CONFIG_RTC



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
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Simple character addressed device */
#define CONFIG_VT_SIMPLE
/* Banked memory set up */
#define CONFIG_BANK_FIXED
#define MAX_MAPS	2
#define MAP_SIZE	0x8000

#define CONFIG_BANKS	2	/* 2 x 32K */

/* Vt definitions */
#define VT_BASE		((uint8_t *)0xF800)
#define VT_WIDTH	80
#define VT_HEIGHT	24
#define VT_RIGHT	79
#define VT_BOTTOM	23

#define TICKSPERSEC 60   /* Ticks per second */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0x7D00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   32 	    /* Memory needed per process */

#define SWAP_SIZE   0x40 	/* 32K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* vectors so its a round number of sectors */

#define MAX_SWAPS	64	/* Should be plenty (2MB!) */

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

