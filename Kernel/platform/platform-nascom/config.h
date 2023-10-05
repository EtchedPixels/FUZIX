/* Set if you want RTC support */
#define CONFIG_RTC
/* RTC is fast */
#define CONFIG_RTC_INTERVAL 1
/* We don't have a clock interrupt */
#define CONFIG_NO_CLOCK
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

#define MAX_MAPS	16
#define MAP_SIZE	0xC000

#define CONFIG_BANKS	1	/* 1 x 60K */

/* Vt definitions */
/* Although it's a simple display the margins and weird top line mean it's
   got its own little driver */
#define VT_WIDTH	48
#define VT_HEIGHT	16	/* Lie for the moment as the top line is weird */
#define VT_RIGHT	47
#define VT_BOTTOM	15

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0xBE00  /* Top of program, udata stash follows */
#define PROC_SIZE   48 	    /* Memory needed per process */

#define SWAP_SIZE   0x60 	/* 48K in blocks (to get the udata stash) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vectors so its a round number of sectors */

#define MAX_SWAPS	64	/* Should be plenty (2MB!) */

#define swap_map(x)	((uint8_t *)(x))

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2	  /* Tackle 80bus serial later */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */
#define CONFIG_DYNAMIC_SWAP	/* Swap file by partition */
#define NBUFS    5       /* Number of block buffers - keep in sync with asm! */
#define NMOUNTS	 2	  /* Number of mounts at a time */

/* Do I/O direct to user space */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* Reclaim the discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

#define CONFIG_TD
#define CONFIG_TD_NUM	2
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_INDIRECT
#define CONFIG_TINYIDE_8BIT
#define IDE_IS_8BIT(x)	1

extern void plt_discard(void);
#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd#"
