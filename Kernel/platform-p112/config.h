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
/* Fixed banking: 8 x 64K banks, top 4KB is shared with kernel */
#define CONFIG_BANK_FIXED
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT
/* 8 60K banks, 1 is kernel */
#define MAX_MAPS	16
#define MAP_SIZE	PROGTOP /* 0xF000 breaks pagemap_realloc() / exec() */

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
#define BOOTDEVICENAMES "hd#,fd"

/* Device parameters */
#define NUM_DEV_TTY 5

#define TTYDEV   (512+1)  /* System console (used by kernel, init) */

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in p112.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Hardware parameters */
#define Z180_IO_BASE       0x00

#define MAX_BLKDEV  2		    /* 2 IDE drives */

#define CONFIG_IDE                  /* enable if IDE interface present */

/* We have a DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Memory backed devices */
#define CONFIG_RAMDISK          /* enable memory-backed device driver */
#define DEV_RD_ROM_SIZE  0      /* size of system ROM in 4KB pages (0KB -- it's unmapped) */
#define DEV_RD_RAM_SIZE  256    /* size of system RAM in 4KB pages (1024KB) */
#define DEV_RD_ROM_PAGES 0      /* size of the ROM disk (/dev/rd1) in 4KB pages */
#define DEV_RD_RAM_PAGES 0      /* size of the RAM disk (/dev/rd0) in 4KB pages */

/* We have the P112 floppy controller */
#define CONFIG_P112_FLOPPY
