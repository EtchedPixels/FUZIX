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
/* Single tasking - for now while we get it booting */
#undef CONFIG_SINGLETASK
#define CONFIG_BANKS	2

/* For now used BANK_FIXED as we don't yet have sane swap with 16K maps */
#define CONFIG_BANK_FIXED
#define MAX_MAPS 2
#define MAP_SIZE 0xE000

/* And swapping */
#define SWAPDEV 	257	/* FIXME */
#define SWAP_SIZE   	0x70 	/* 56K in blocks */
#define SWAPBASE    	0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    	0xE000	/* vectors so its a round number of sectors */
/* FIXME: we need to swap the udata separately */
#define MAX_SWAPS	32

#define TICKSPERSEC 10	    /* Ticks per second */
#define PROGBASE    0x2000  /* also data base */
#define PROGLOAD    0x2000
#define PROGTOP     0xE000  /* Top of program */

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    8        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
