/* Set if you want RTC support and have an RTC on ports 0xB0-0xBC */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL	10
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
/* Switchable console */
#define CONFIG_VT_MULTI
/* Banked memory set up */
#define CONFIG_BANK_FIXED
/* Direct I/O support */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* Raw input layer */
#define CONFIG_INPUT
/* Full keycode level grabbing supported */
#define CONFIG_INPUT_GRABMAX	3
/* External buffers (so we can balance things better) */
#define CONFIG_BLKBUF_EXTERNAL
/* And our buffer pool is dynamically sized */
#define CONFIG_DYNAMIC_BUFPOOL
/* And networking (to revisit once the new net code is more sorted) */
#undef CONFIG_NET
#undef CONFIG_NET_NATIVE
/* And IDE */
#define CONFIG_TD_NUM	4
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
#define IDE_IS_8BIT(x)	1

#define MAX_MAPS	16	/* 512K */

#define MAP_SIZE	0x8000

#define CONFIG_BANKS	2	/* 2 x 32K */

/* Vt definitions */
#define VT_WIDTH	64
#define VT_HEIGHT	16
#define VT_RIGHT	63
#define VT_BOTTOM	15

/* Keyboard bitmap definitions */
#define KEY_ROWS	8
#define KEY_COLS	8

#define TICKSPERSEC 40	    /* Ticks per second */
#define PROGBASE    0x8000  /* Base of user  */
#define PROGLOAD    0x8000  /* Load and run here */
#define PROGTOP     0xFE00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   32 	    /* Memory needed per process */

#define SWAP_SIZE   0x40 	/* 32K in blocks */
#define SWAPBASE    0x8000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x10000U	/* udata cache at the top */

#define MAX_SWAPS	16	/* Should be plenty (512K!) */

#define swap_map(x)	((uint8_t *)(x))

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 4
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */
#define NBUFS    5         /* Number of block buffers - keep in sync with asm! */
#define NMOUNTS	 3	   /* Number of mounts at a time */

extern void plt_discard(void);
#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd#"
