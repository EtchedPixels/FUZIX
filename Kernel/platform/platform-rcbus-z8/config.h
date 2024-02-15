/* We have an RTC */
#undef CONFIG_RTC
/* And we can read ToD from it */
#undef CONFIG_RTC_FULL
/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 2 32K banks, + 64K is kernel */
/* When we run from flash we'll fix this to 2 x 64K */
#define MAX_MAPS	2
#define MAP_SIZE	0x8000U

#define CONFIG_TD_NUM	1
#define CONFIG_TD_SD
#define TD_SD_NUM	1

/* Read processes and big I/O direct into process space */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 200   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xBE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   48	  /* Memory needed per process */

/* Swap not enabled yet */
#define SWAP_SIZE   0x60 	/* 48K in blocks (we actually don't need the low 256) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vectors so its a round number of sectors */
#define MAX_SWAPS   16		/* The full drive would actually be 85! */

#define swap_map(x)	((uint8_t *)(x)) /* Simple zero based mapping */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#if 0
#define SWAPDEV  (256 + 1)  /* Device for swapping. (FIXME) */
#endif
#define NBUFS    8	  /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
