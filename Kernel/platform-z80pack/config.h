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
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 8 60K banks, 1 is kernel */
#define MAX_MAPS	7
#define MAP_SIZE	0xF000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGLOAD    ((char *)(0x0000))  /* also data base */
#define PROGBASE    ((char *)(0x0100))  /* also data base */
#define PROGTOP     ((char *)(0xED00))  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   60	  /* Memory needed per process */

#define SWAP_SIZE   0x78 	/* 60K in blocks (we actually don't need the low 256) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xF000	/* vectors so its a round number of sectors */
#define MAX_SWAPS	64	/* The full drive would actually be 85! */

#define UDATA_BLOCKS	0	/* We swap the stash not the uarea */
#define UDATA_SWAPSIZE	0

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
