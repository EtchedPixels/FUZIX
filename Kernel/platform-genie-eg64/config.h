/* Set if you want RTC support and have an RTC on ports 0xB0-0xBC */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL	10	/* fast to read */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Memory set up */
#define CONFIG_SWAP_ONLY
#define CONFIG_PARENT_FIRST
#define CONFIG_SPLIT_UDATA
#define UDATA_BLKS 1
#define UDATA_SIZE 0x200
#define MAXTICKS 100
/* Direct I/O support */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* Raw input layer */
#define CONFIG_INPUT
/* Full keycode level grabbing supported */
#define CONFIG_INPUT_GRABMAX	3
/* And our buffer pool is dynamically sized */
#define CONFIG_DYNAMIC_BUFPOOL

#define MAP_SIZE	0xA000

#define CONFIG_BANKS	1	/* 32K */

/* Vt definitions */
#define CONFIG_VT_SIMPLE
#define VT_BASE		((uint8_t *)0x3C00)
#define VT_MAP_CHAR(x)	vt_map_char(x)
#define VT_WIDTH	64
#define VT_HEIGHT	16
#define VT_RIGHT	63
#define VT_BOTTOM	15

extern unsigned char vt_map_char(unsigned char);

/* Keyboard bitmap definitions */
#define KEY_ROWS	8
#define KEY_COLS	8

#define TICKSPERSEC 40	    /* Ticks per second */
#define PROGBASE    0x6000  /* Base of user  */
#define PROGLOAD    0x6000  /* Load and run here */
#define PROGTOP     0xFFFF  /* Top of program */
#define PROC_SIZE   40 	    /* Memory needed per process */

#define SWAP_SIZE   0x51 	/* 44.5K in blocks */
#define SWAPBASE    0x6000	/* We swap the lot in one */
#define SWAPTOP	    0x10000UL

#define MAX_SWAPS	16	/* Should be plenty (512K!) */

#define swap_map(x)	((uint8_t *)(x))

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */
#define NBUFS    4         /* Number of block buffers - keep in sync with asm! */
#define NMOUNTS	 4	   /* Number of mounts at a time */

extern void plt_discard(void);
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
