/* Set if you want RTC support and have an RTC on ports 0xB0-0xBC */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL	10	/* fast RTC */
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
/* Simple character addressed device */
#define CONFIG_VT_SIMPLE
#define CONFIG_VT_MULTI
/* Banked memory set up */
#define CONFIG_BANK_FIXED
/* Input device support */
#define CONFIG_INPUT
/* Full key up/down support */
#define CONFIG_INPUT_GRABMAX 3

#define MAX_MAPS	62

#define MAP_SIZE	0x8000

#define CONFIG_BANKS	2	/* 2 x 32K */

/* Vt definitions */
#define VT_BASE		vtbase[curtty]
#define VT_WIDTH	80
#define VT_HEIGHT	24
#define VT_RIGHT	79
#define VT_BOTTOM	23

#define TICKSPERSEC 60   /* Ticks per second */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0x7E00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   32 	    /* Memory needed per process */

#define SWAP_SIZE   0x40 	/* 32K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* vectors so its a round number of sectors */

#define MAX_SWAPS	64	/* Should be plenty (2MB!) */

#define swap_map(x)	((uint8_t *)(x))

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */
#define NBUFS    5        /* Number of block buffers - keep in sync with asm! */
#define NMOUNTS	 4	  /* Number of mounts at a time */
/* Reclaim the discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL
/* Use large I/O */
#define CONFIG_LARGE_IO_DIRECT(x)	1

extern void plt_discard(void);
#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd#"

#define CONFIG_SMALL
