/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Swap based one process in RAM */
#define CONFIG_SWAP_ONLY
#define CONFIG_PARENT_FIRST
#define MAXTICKS 20
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* One memory bank */
#define CONFIG_BANKS	1
#define TICKSPERSEC 10      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program */
#define KERNTOP	    0xF000  /* Grow buffers to 0xF000 */

#define PROC_SIZE   61	  /* Memory needed per process (inc udata) */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x7A 	/* 61K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF000	/* Swap out program */
#define CONFIG_SPLIT_UDATA	/* Adjacent addresses but different bank! */
#define UDATA_BLKS  1		/* One block of udata */
#define UDATA_SIZE  512		/* 512 bytes of udata */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP

/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. For a simple banked setup there is no conversion
 *	needed so identity map it.
 */
#define swap_map(x)	((uint8_t *)(x))

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define MAX_BLKDEV 4	    /* 1 ROM disk, 1 RAM disk, 1 floppy, 1 IDE */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_NO_CLOCK
/* Set how often we actually poll this RTC in ticks - 1 means always. On the
   SC108 it's slow so don't sync often. If we have no timer tick then we will
   read the RTC regularly as needed - and it'll suck accordingly regardless
   of this setting */
#define CONFIG_RTC_INTERVAL	100

#define CONFIG_TD_NUM		2
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
#define IDE_IS_8BIT(x)		1

/* Device parameters */
#define NUM_DEV_TTY 4

/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define plt_copyright()
