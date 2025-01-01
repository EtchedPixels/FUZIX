/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* 32 x 16K pages, 3 pages for kernel, whatever the RAM disk uses */
#define MAX_MAPS	(32 - 3 - DEV_RD_RAM_PAGES)

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 20      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */
#define KERNTOP     0xC000  /* Top of kernel (first 3 banks), base of shared bank */
#define PROC_SIZE   64	    /* Memory needed per process */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x79 	/* 63.5K in blocks (which is the wrong number) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF200	/* can we stop at the top? not sure how. let's stop short. */
#define MAX_SWAPS   10		/* 10 + RAM processes */
#define CONFIG_DYNAMIC_SWAP	/* Swap device is discovered by partition */
/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. We use the second 16K window
 */
#define swap_map(x)	((uint8_t *)((((x) & 0x3FFF)) + 0x4000))

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd,,rd"

/* #define SWAPDEV  (256 + 1) */  /* Device for swapping */
#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4	    /* 1 ROM disk, 1 RAM disk, 1 floppy, 1 PPIDE */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Floppy support */
#define CONFIG_FLOPPY		/* #define CONFIG_FLOPPY to enable floppy */

/* PPIDE support */
#define CONFIG_TD_NUM		2
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_INDIRECT
#define CONFIG_TINYIDE_PPI

/* Device parameters */
#define CONFIG_DEV_MEM          /* enable /dev/mem driver */

#define CONFIG_RAMDISK          /* enable memory-backed disk driver */
#define DEV_RD_ROM_PAGES 28     /* size of the ROM disk in 16KB pages (max 32, any unused pages are at the start of the ROM) */
#define DEV_RD_RAM_PAGES 0      /* size of the RAM disk in 16KB pages */

#define DEV_RD_ROM_START ((uint32_t)(32-DEV_RD_ROM_PAGES) << 14)        /* first byte used by the ROM disk */
#define DEV_RD_RAM_START ((uint32_t)(64-DEV_RD_RAM_PAGES) << 14)        /* first byte used by the RAM disk */
#define DEV_RD_ROM_SIZE  ((uint32_t)DEV_RD_ROM_PAGES << 14)             /* size of the ROM disk */
#define DEV_RD_RAM_SIZE  ((uint32_t)DEV_RD_RAM_PAGES << 14)             /* size of the RAM disk */

#define NUM_DEV_TTY 1

/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B38400

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define plt_copyright()		/* For now */
