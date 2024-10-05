/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Acct syscall support */
#undef CONFIG_ACCT
/* Multiple processes in memory at once */
#define CONFIG_MULTI

/* 2K reported page size */
#define CONFIG_BANKS		16
#define CONFIG_PAGE_SIZE	2

/* Custom pager */

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

#define TICKSPERSEC 20	    /* Ticks per second */

#define MAPBASE	    0x0100  /* We map from 0x0100 */
#define PROGBASE    0x0100  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xE100

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 4
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"

#define BLKSIZE	400	  /* Physical media has 400 byte blocks */
