/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* CP/M emulation */
#define CONFIG_CPM_EMU
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Multiple VT support */
#define CONFIG_VT_MULTI
#define MAX_VT	5
#define CONFIG_FONT6X8
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 16 48K banks, 1 is kernel */
#define MAX_MAPS	16
#define MAP_SIZE	0xC000U

#define CONFIG_LARGE_IO_DIRECT(x)	((x) != 1)
/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 60      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xBE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   48	    /* Memory needed per process */

#define SWAP_SIZE   0x60 	/* 48K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vectors so its a round number of sectors */
#define MAX_SWAPS   64		/* How many swaps per disc */
#define SWAPDEV  (swap_dev)     /* Device for swapping. */

extern uint16_t swap_dev;

#define CONFIG_DYNAMIC_SWAP	/* Find swap partitions on disks */
#define CONFIG_PLATFORM_SWAPCTL	/* Because we need to be able to swap on
                                   silicon discs and can't autodecide this */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 7	/* 4 VDP, 1 80 col, 2 serial */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

/* We will size the buffer pool to fill the space */
#define CONFIG_DYNAMIC_BUFPOOL
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 3	  /* Number of mounts at a time */

/* Terminal definitions */
#define VT_WIDTH	vt_twidth[outputtty + 1]
#define VT_HEIGHT	24
#define VT_RIGHT	vt_tright[outputtty + 1]
#define VT_BOTTOM	23

#define CONFIG_IDE
#define CONFIG_PPIDE
#define MAX_BLKDEV	2

#define CONFIG_SD
#define SD_DRIVE_COUNT	1

#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL 10		/* Fast RTC */

/* Input device support */
#define CONFIG_INPUT
/* Full key up/down support */
#define CONFIG_INPUT_GRABMAX 3

#define swap_map(x)	((uint8_t *)(x))

#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd"
