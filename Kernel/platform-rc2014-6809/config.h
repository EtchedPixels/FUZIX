/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Pure swap */
#undef CONFIG_SWAP_ONLY

#define CONFIG_BANK_FIXED
#define MAX_MAPS 8
#define MAP_SIZE 0xBE00U
#define CONFIG_BANKS	1
/* And swapping */
#define SWAPDEV (swap_dev)	/* Dynamic swap */
#define SWAP_SIZE   0x60	/* 48K in 512 byte blocks */
#define SWAPBASE    0x0000	/* We swap the lot, including stashed uarea */
#define SWAPTOP     0xC000	/* so it's a round number of 256 byte sectors */
#define MAX_SWAPS   32
#define CONFIG_DYNAMIC_SWAP

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Reclaim the discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

#define MAX_BLKDEV  	2	/* 2 IDE drives */
#define CONFIG_IDE              /* enable if IDE interface present */

#define TICKSPERSEC 10   /* Ticks per second */

#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0000  /* also data base */
#define PROGTOP     0xBE00  /* Top of program */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers at boot time */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define swap_map(x)	((uint8_t *)(x))

extern void platform_discard(void);

#define platform_copyright()		/* for now */

#define BOOTDEVICENAMES "hd#"
