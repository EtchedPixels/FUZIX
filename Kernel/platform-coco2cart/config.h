/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Pure swap */
#define CONFIG_SWAP_ONLY
#define CONFIG_SPLIT_UDATA
#define UDATA_BLKS 1

#define CONFIG_BANKS	1
/* And swapping */

extern uint16_t cocoswap_dev;

#define SWAPDEV     cocoswap_dev    /* Uses part of IDE slice 0 or SD 0*/
#define SWAP_SIZE   0x40	/* 32K in 512 byte blocks */
#define SWAPBASE    0x8000	/* We swap the lot */
#define SWAPTOP     0xFE00	/* so it's a round number of 512 byte sectors */
#define UDATA_SIZE  0x0200	/* one block */
#define MAX_SWAPS   32

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL

/* We can reclaim discard into buffers */
#define CONFIG_DYNAMIC_BUFPOOL

/* Allow for our swap heavy nature */
#define MAXTICKS 25

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23
#define VT_INITIAL_LINE	0

#define VIDEO_BASE	0x0200

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x8000  /* also data base */
#define PROGLOAD    0x8000  /* also data base */
#define PROGTOP     0xFE00  /* Top of program */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define NDEVS    2        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   513	 /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define swap_map(x)	((uint8_t *)(x))

#define CONFIG_IDE
#define platform_copyright()

#define BOOTDEVICENAMES "hd#"
