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
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* 12 60K banks, 1 is kernel, set to the number of banks available for user programs */
#define MAX_MAPS    11
#define MAP_SIZE    PROGTOP /* WRS: I feel this should be 60KB, but setting it so breaks pagemap_realloc() when exec calls it */

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 40U     /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF800  /* Top of program, base of U_DATA copy */
#define KERNTOP     0xF000  /* Kernel has lower 60KB */
#define PROC_SIZE   64      /* Memory needed per process */

/* We need a tidier way to do this from the loader */
#define CMDLINE	(0x0081)    /* Location of root dev name */
#define BOOTDEVICENAMES "hd#"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS       4       /* Number of block buffers, keep in line with space reserved in yaz180.s */
#define NMOUNTS     4       /* Number of mounts at a time */

/* Hardware parameters */
#define Z180_IO_BASE       0x00

#define MAX_BLKDEV  1       /* 1 IDE drive */

#define CONFIG_IDE
#define CONFIG_PPIDE

#define NUM_DEV_TTY 2
/* ASCI0 as the console */
#define TTYDEV   (512+1)    /* System console (used by kernel, init) */

#define platform_copyright()
