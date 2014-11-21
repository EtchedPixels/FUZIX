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
/* Fixed banking */
#undef CONFIG_BANK_FIXED
/* Swap only */
#define CONFIG_SWAP_ONLY
/* Simple user copies */
#define CONFIG_USERMEM_C
#define BANK_KERNEL
#define BANK_PROCESS

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    ((char *)(0x0000))  /* also data base */
#define PROGLOAD    ((char *)(0x0100))
#define PROGTOP     ((char *)(0x7D00))  /* Top of program, base of U_DATA */
#define PROC_SIZE   32		  /* Memory needed per process */

#define SWAP_SIZE   0x40 	/* 32K in blocks (we actually don't need the low 256) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* vectors so its a round number of sectors */
#define MAX_SWAPS	64	/* The full drive would actually be 170! */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (256 + 1)  /* Device for swapping. (z80pack drive J) */
#define NBUFS    6        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
