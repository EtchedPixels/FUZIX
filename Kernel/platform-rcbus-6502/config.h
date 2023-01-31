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
/* Use fixed banks for now. It's simplest and we've got so much memory ! */
#define CONFIG_BANKS	1
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

#define CONFIG_CALL_R2L		/* Runtime stacks arguments backwards */

/*
 *	512K RAM (swap yet to do )
 *	Question: is common better top or bottom ?
 *	Top means we switch ZP and 6502 stacks, bottom means we don't but
 *	have to copy stuff/watching sharing
 */
#define CONFIG_BANK_FIXED
#define MAX_MAPS 	7   /* 9 x 64K */
#define MAP_SIZE    0xDE00

#define TICKSPERSEC 100	    /* Ticks per second */

/* We've not yet made the rest of the code - eg tricks match this ! */
#define MAPBASE	    0x0000  /* We map from 0 */
#define PROGBASE    0x2000  /* also data base */
#define PROGLOAD    0x2000
#define PROGTOP     0xFE00

#define CONFIG_IDE
#define MAX_BLKDEV 1

/* FIXME: swap */

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
