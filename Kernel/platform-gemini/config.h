/* Set if you want RTC support */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 100
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
/* Input device support */
#define CONFIG_INPUT
/* Full key up/down support */
#define CONFIG_INPUT_GRABMAX 3
/* SCSI or SASI */
#define CONFIG_SCSI

#define MAX_MAPS	16
#define MAP_SIZE	0xE600

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
#define PROGTOP     0xE600  /* Top of program, udata stash follows */
#define PROC_SIZE   58 	    /* Memory needed per process */

#define SWAP_SIZE   0x74 	/* 58K in blocks (to get the udata stash) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xE600	/* vectors so its a round number of sectors */

#define MAX_SWAPS	64	/* Should be plenty (2MB!) */

#define swap_map(x)	((uint8_t *)(x))

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

#define MAX_BLKDEV	4

/* Device parameters */
#define NUM_DEV_TTY 2	  /* Tackle 80bus serial later */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */
#define NBUFS    10       /* Number of block buffers - keep in sync with asm! */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Do I/O direct to user space */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* Reclaim the discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

extern void plt_discard(void);
#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd#"
