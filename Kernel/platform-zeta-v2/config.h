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
#define PROC_SIZE   64	  /* Memory needed per process */

/* WRS: this is probably wrong -- we want to swap the full 64K minus the common code */
/* For now let's just use something and fix this up later when we have a swap device */
#define SWAP_SIZE   0x7F 	/* 63.5K in blocks (which is the wrong number) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xFF00	/* can we stop at the top? not sure how. let's stop short. */
#define MAX_SWAPS	10	    /* Well, that depends really, hmmmmmm. Pick a number, any number. */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd,,rd"

//#define SWAPDEV  (256 + 1)  /* Device for swapping */
#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4	    /* 1 ROM disk, 1 RAM disk, 1 floppy, 1 PPIDE */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Floppy support */
#define CONFIG_FLOPPY		/* #define CONFIG_FLOPPY to enable floppy */

/* PPIDE support */
#define CONFIG_PPIDE 		/* #define CONFIG_PPIDE to enable IDE on 8255A */
#define CONFIG_IDE		/* required for CONFIG_PPIDE */

/* Optional ParPortProp board connected to PPI */
//#define CONFIG_PPP		/* #define CONFIG_PPP to enable as tty3 */

/* Device parameters */
#define CONFIG_DEV_MEM          /* enable /dev/mem driver */

#define CONFIG_RAMDISK          /* enable memory-backed disk driver */
#define DEV_RD_ROM_PAGES 28     /* size of the ROM disk in 16KB pages (max 32, any unused pages are at the start of the ROM) */
#define DEV_RD_RAM_PAGES 0      /* size of the RAM disk in 16KB pages */

#define DEV_RD_ROM_START ((uint32_t)(32-DEV_RD_ROM_PAGES) << 14)        /* first byte used by the ROM disk */
#define DEV_RD_RAM_START ((uint32_t)(64-DEV_RD_RAM_PAGES) << 14)        /* first byte used by the RAM disk */
#define DEV_RD_ROM_SIZE  ((uint32_t)DEV_RD_ROM_PAGES << 14)             /* size of the ROM disk */
#define DEV_RD_RAM_SIZE  ((uint32_t)DEV_RD_RAM_PAGES << 14)             /* size of the RAM disk */

#ifdef CONFIG_PPP
	/* SD card in ParPortProp */
	#define CONFIG_SD
        #define SD_DRIVE_COUNT 1
	#define NUM_DEV_TTY 2

	/* ParPortProp as the console */
	#define BOOT_TTY (512 + 2)
#else
	#define NUM_DEV_TTY 1

	/* UART0 as the console */
	#define BOOT_TTY (512 + 1)
	#define TTY_INIT_BAUD B38400
#endif

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define platform_copyright()		/* For now */
