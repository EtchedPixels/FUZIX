/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* 32K banking */
#define CONFIG_BANK32
/* but with the high block copied on switch as needed */
#define CONFIG_COMMON_COPY
/* 8 32K banks, 1 is kernel */
#define MAX_MAPS	7
#define MAP_SIZE	0x8000U

/* Banks as reported to user space */
#define CONFIG_BANKS	2

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xBC00  /* Top of program, base of U_DATA copy */

#define SWAP_SIZE   0x60 	/* 48K in blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vectors so its a round number of sectors */
#define MAX_SWAPS	64	/* The full drive would actually be 85! */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (256 + 1)  /* Device for swapping. (z80pack drive J) */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
