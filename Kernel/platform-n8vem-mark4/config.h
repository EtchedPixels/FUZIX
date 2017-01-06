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
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* Fixed banking: 8 x 64K banks, top 4KB is shared with kernel, 60KB-62KB is user memory  */
#define CONFIG_BANK_FIXED
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT
/* 8 60K banks, 1 is kernel */
#define MAX_MAPS	8
#define MAP_SIZE	PROGTOP    /* WRS: I feel this should be 60KB, but setting it so breaks pagemap_realloc() when exec calls it */

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 40U     /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF800  /* Top of program, base of U_DATA copy */
#define KERNTOP     0xF000  /* Kernel has lower 60KB */
#define PROC_SIZE   64      /* Memory needed per process */

/* We need a tidier way to do this from the loader */
#define CMDLINE	(0x0081)  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,,,rd"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in mark4.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Hardware parameters */
#define Z180_IO_BASE       0x40
#define MARK4_IO_BASE      0x80

#define MAX_BLKDEV 3	    /* 2 IDE drives, 1 SD drive */

#define CONFIG_IDE
#define IDE_REG_BASE       MARK4_IO_BASE
#define IDE_8BIT_ONLY
#define IDE_REG_CS1_FIRST

/* On-board SD on Mark IV */
#define CONFIG_SD
#define SD_DRIVE_COUNT 1

/* On-board DS1302 on Mark IV, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Memory backed devices */
#define CONFIG_RAMDISK          /* enable memory-backed device driver */
#define DEV_RD_ROM_SIZE  128    /* size of system ROM in 4KB pages (512KB) */
#define DEV_RD_RAM_SIZE  128    /* size of system RAM in 4KB pages (512KB) */
#define DEV_RD_ROM_PAGES 64     /* size of the ROM disk (/dev/rd1) in 4KB pages */
#define DEV_RD_RAM_PAGES 0      /* size of the RAM disk (/dev/rd0) in 4KB pages */

/* Optional PropIOv2 board on ECB bus */
//#define CONFIG_PROPIO2		/* #define CONFIG_PROPIO2 to enable as tty3 */
#define PROPIO2_IO_BASE		0xA8

/* Device parameters */
#ifdef CONFIG_PROPIO2
	#define NUM_DEV_TTY 3

	/* PropIO as the console */
	#define TTYDEV   (512+3)  /* System console (used by kernel, init) */
#else
	#define NUM_DEV_TTY 2

	/* ASCI0 as the console */
	#define TTYDEV   (512+1)  /* System console (used by kernel, init) */
#endif
