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

#define CONFIG_CALL_R2L		/* Runtime stacks arguments backwards */

/*
 *	As we have 1MB of RAM we simply allocate it into 64K per process
 *	for 15 processes plus kernel. A big server would no doubt want to
 *	use 16K banks and swapping 8)
 */
#define CONFIG_BANK_FIXED
#define MAX_MAPS 	15
#define MAP_SIZE    0xE000  /* The low 8K is taken up with common space ZP and
                               S, while we don't currently use the top 8K
                               (see tricks.s and fix up the fork copy code) */

#define TICKSPERSEC 10	    /* Ticks per second */
#define MAPBASE	    0x0000  /* We map from 0 */
#define PROGBASE    0x2000  /* also data base */
#define PROGLOAD    0x2000
#define PROGTOP     0xE000  /* Top of program. If we fixed a few things we
                               could go to FE00 */

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    8        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define platform_discard()
#define platform_copyright()

#define BOOTDEVICENAMES "hd#"
