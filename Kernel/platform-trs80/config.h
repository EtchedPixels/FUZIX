/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Single tasking */
#define CONFIG_SINGLETASK
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Simple character addressed device */
#define CONFIG_VT_SIMPLE

#define CONFIG_BANKS	2	/* 2 x 32K */

/* Vt definitions */
#define VT_BASE		((uint8_t *)0xF800)
#define VT_WIDTH	80
#define VT_HEIGHT	25
#define VT_RIGHT	79
#define VT_BOTTOM	24

#define TICKSPERSEC 60   /* Ticks per second */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0xF900  /* Top of program, base of U_DATA */
#define PROC_SIZE   64	  /* Memory needed per process */

#define SWAP_SIZE   0x80 	/* 64K in blocks (we actually don't need quite all) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xF400	/* vectors so its a round number of sectors */

#define MAX_SWAPS	8	/* Should be plenty */

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3
#define NDEVS    17       /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  5	  /* Device for swapping. */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */





