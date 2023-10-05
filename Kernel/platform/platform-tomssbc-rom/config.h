#define CONFIG_LARGE_IO_DIRECT(x)	1  /* We support direct to user I/O */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
#define CONFIG_SWAP_ONLY
/* CP/M emulation */
#undef CONFIG_CPM_EMU

/* Swap only mode for Z80 requires this */
#define CONFIG_PARENT_FIRST

/* Settings for swap only mode */
#define CONFIG_SPLIT_UDATA
#define UDATA_SIZE 0x200
#define UDATA_BLKS 1

#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_DYNAMIC_SWAP

/* We have one mapping from our working of memory */
#define MAX_MAPS	1
#define MAP_SIZE	0xC000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 10   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xC000  /* Top of program */
#define PROC_SIZE   48	  /* Memory needed per process */

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY	2
#define CONFIG_TD_NUM	4
/* RC2014 style CF IDE */
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
#define IDE_NONSTANDARD_XFER	/* As we are banked use own helpers */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define MAX_BLKDEV 2	    /* 2 IDE drives */

#define SWAPBASE 0x0000
#define SWAPTOP  0xC000
#define SWAP_SIZE 0x61

#define MAX_SWAPS	14
#define SWAPDEV (swap_dev)

/* All our swap is to direct mapped space */
#define swap_map(x)		((uint8_t *)(x))

#define BOOTDEVICENAMES "hd#"
