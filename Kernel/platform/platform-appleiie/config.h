/* #define CONFIG_RAMWORKS */

#ifdef CONFIG_RAMWORKS
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Use fixed banks for now. It's simplest and we've got so much memory ! */
#define CONFIG_BANK_FIXED
#define MAX_MAPS 	15
#define MAP_SIZE    0xB800  
#else
/* One process in memory for base 128K system - 6502 just isnt compact enough
   for a two process setup as Z80 would do for 128K */
#define CONFIG_BANKS	1	/* 1 bank per process */
#define CONFIG_MULTI		/* Multi-tasking */
#define CONFIG_SWAP_ONLY	/* One process in memory rest by swap */
#define CONFIG_SPLIT_UDATA	/* We'll need to do some work on this.. */
#define UDATA_SIZE  512
#define UDATA_BLKS  1

/* HACK for now */
#define SWAPDEV	(1 << 5)
#endif


/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Acct syscall support */
#undef CONFIG_ACCT

#define CONFIG_CALL_R2L		/* Runtime stacks arguments backwards */

#define TICKSPERSEC 50	    /* Ticks per second. Needs work as will vary */
#define MAPBASE	    0x0800  /* We map from 0x0800 */
#define PROGBASE    0x0800  /* also data base */
#define PROGLOAD    0x0800
#define PROGTOP     0xBE00  /* When we hit the data space */

#define SWAP_SIZE   0x5F	/* 47.5K - allow for udata and our magic */
#define SWAPBASE    0x0800	/* FIXME: will need to swap ZP and stack too */
#define SWAPTOP	    0xBE00
#define MAX_SWAPS   PTABSIZE
#define swap_map(x)	((uint8_t *)(x))

/* Support a 40 column console for now */
#define CONFIG_VT
#define VT_RIGHT 39
#define VT_BOTTOM 24

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1	  /* For now until we add console switches */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 3	  /* Number of mounts at a time */

#define CONFIG_SMALL

#define plt_discard()
#define plt_copyright()
