/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed banking: 8 x 64K banks, top 4KB is shared with kernel, 60KB-62KB is user memory  */
#define CONFIG_BANK_FIXED
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
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

#define BOOTDEVICENAMES "hd#,,,rd"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in mark4.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Hardware parameters */
#define Z180_IO_BASE       0x40

#define MAX_BLKDEV 3	    /* 1 SD drive for now */

/* On-board SD on the N8 */
#define CONFIG_SD
#define SD_DRIVE_COUNT 1

/* On-board DS1302 on Mark IV, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

#define NUM_DEV_TTY 2		/* Until we tackle the VDP */

/* ASCI0 as the console */
#define TTYDEV   (512+1)  /* System console (used by kernel, init) */

#define platform_copyright()

#define CMDLINE NULL