/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT
/* 32 x 16K pages, 3 pages for kernel, whatever the RAM disk uses */
#define MAX_MAPS	(32 - 3 - DEV_RD_RAM_PAGES)

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 20      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */
#define KERNTOP     0xC000  /* Top of kernel (first 3 banks), base of shared bank */
#define PROC_SIZE   64	  /* Memory needed per process */

/* WRS: this is probably wrong -- we want to swap the full 64K minus the common code */
/* For now let's just use something and fix this up later when we have a swap device */
#define SWAP_SIZE   0x79 	/* 60.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF200	/* Swap out udata and program */
/* FIXME: do this off partition tables */
#define MAX_SWAPS   10	    	/* Well, that depends really, hmmmmmm. Pick a number, any number. */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd,,rd"

//#define SWAPDEV  (256 + 1)  /* Device for swapping */
#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4	    /* 1 ROM disk, 1 RAM disk, 1 floppy, 1 IDE */

#if 0	/* for now */
/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */
#endif

/* Floppy support */
#define CONFIG_FLOPPY		/* #define CONFIG_FLOPPY to enable floppy */
/* IDE/CF support */
#define CONFIG_IDE

#define CONFIG_VFD_TERM         /* #define CONFIG_VFD_TERM to show console output on VFD display */

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	0	/* No keyboard to grab */

/* Device parameters */
#define CONFIG_DEV_MEM          /* enable /dev/mem driver */

#define CONFIG_RAMDISK          /* enable memory-backed disk driver */
#define DEV_RD_ROM_PAGES 28     /* size of the ROM disk in 16KB pages (max 32, any unused pages are at the start of the ROM) */
#define DEV_RD_RAM_PAGES 0      /* size of the RAM disk in 16KB pages */

#define DEV_RD_ROM_START ((uint32_t)(32-DEV_RD_ROM_PAGES) << 14)        /* first byte used by the ROM disk */
#define DEV_RD_RAM_START ((uint32_t)(64-DEV_RD_RAM_PAGES) << 14)        /* first byte used by the RAM disk */
#define DEV_RD_ROM_SIZE  ((uint32_t)DEV_RD_ROM_PAGES << 14)             /* size of the ROM disk */
#define DEV_RD_RAM_SIZE  ((uint32_t)DEV_RD_RAM_PAGES << 14)             /* size of the RAM disk */

#define NUM_DEV_TTY 2

/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

/* The Scott Baker SIO has a non-standard layout (it predates the official one) */
/* You'll need to define this if you have a Scott Baker SIO2 card, or submit
   a fancier autodetect! Also you'll need to change rc2014.s */
#undef CONFIG_SIO_BAKER
