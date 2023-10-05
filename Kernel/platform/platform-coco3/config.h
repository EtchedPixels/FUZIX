/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Use C helpers for usermem */
#undef CONFIG_USERMEM_C

/* Reclaim discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* We use flexible 16K banks so use the helper */
#define CONFIG_BANK16
#define CONFIG_BANKS	4
#define MAX_MAPS 128-3
#define MAPBASE 0x0000
/* And swapping */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
extern uint16_t swap_dev;
#define SWAPDEV  swap_dev
#define SWAP_SIZE   0x71
/* FIXME */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xe200	/* uarea so its a round number of sectors */
#define UDATA_BLOCKS	0	/* We swap the uarea in the data */
#define UDATA_SWAPSIZE	0
#define MAX_SWAPS	32
#define swap_map(x)  ((uint8_t *)(x & 0x3fff ))


/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_VT_MULTI
#define VT_BASE      (uint8_t *)0xb400
#define VT_WIDTH	curtty->width
#define VT_HEIGHT	curtty->height
#define VT_RIGHT	curtty->right
#define VT_BOTTOM	curtty->bottom
#define VT_INITIAL_LINE 0
#define	TTYSIZE	    256		/* Banked so we can afford full size buffers */

#define TICKSPERSEC 60		/* Ticks per second */
#define PROGBASE    0x0100	/* also data base */
#define PROGTOP     0xe000	/* Top of program, base of U_DATA */
#define PROGLOAD    0x0100	/* ??? */

#define BOOT_TTY (512 + 1)	/* Set this to default device for stdio, stderr */
			  /* In this case, the default is the first TTY device */
			    /* Temp FIXME set to serial port for debug ease */

/* Boot devices */
#define BOOTDEVICENAMES "hd#,,,,,,,,dw"



/* We need a tidier way to do this from the loader */
#define CMDLINE	0x88		/* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 10
#define TTYDEV   BOOT_TTY	/* Device used by kernel for messages, panics */
#define NBUFS    6		/* Number of block buffers */
#define NMOUNTS	 4		/* Number of mounts at a time */

/* Drivewire Defines */
#define DW_VSER_NUM 4		/* No of Virtual Serial Ports */
#define DW_VWIN_NUM 4		/* No of Virtual Window Ports */
#define DW_MIN_OFF  3		/* Minor number offset */

/* Block device define */
#define CONFIG_TD_NUM	4		/* 2 IDE + 2 SDC */
#define CONFIG_TD_IDE
#define CONFIG_TD_SD
#define TD_SD_NUM	2
#define CONFIG_TD_CUSTOMPART		/* Custom partition tables (CCPT) */

#define CONFIG_RTC		/* enable RTC code */
#define CONFIG_RTC_INTERVAL 100	/* time in deciseconds to atually poll rtc */

/* Level 2 groups, coredumps, network */
#undef CONFIG_LEVEL_2
#define CONFIG_NET
#define CONFIG_NET_NATIVE

/* redefine tty queue primitives to use our banking ones */
void putq(unsigned char *ptr, char c);
unsigned char getq(unsigned char *ptr);
#define CONFIG_INDIRECT_QUEUES
typedef unsigned char *queueptr_t;
#define GETQ(p) getq(p)
#define PUTQ(p, v) putq((p), (v))

#define CONFIG_DEV_PLATFORM
