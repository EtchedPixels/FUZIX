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

/* Networking (not usable yet but for debug/development) */
#undef CONFIG_NET
#undef CONFIG_NET_NATIVE

#define CONFIG_VT
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL	/* for now until fix loaders */
#define VT_RIGHT 79
#define VT_BOTTOM 24

/*
 *	We have 512K of RAM and have to allocate it in banks due to the CPU
 *	bank granularity. That gives us 7 processes plus kernel and more
 *	if we add swap.
 */
#define CONFIG_BANK_65C816
#define KERNEL_BANK	0
#define MAX_MAPS 	7
#define MAP_SIZE    0xFC00  /* 0-FBFF */

#define CONFIG_LARGE_IO_DIRECT(x)	1

/* 0xEE because our first bank is 1 and 0xEE + 2 * 1 = 0xF0 */
#define STACK_BANKOFF	0xEE	/* F000-FDFF */

#define TICKSPERSEC 100	    /* Ticks per second */
#define MAPBASE	    0x0000  /* We map from 0 */
#define PROGBASE    0x0100  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xFC00  /* Top of program. If we fixed a few things we
                               could go to FE00 */

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define plt_discard()	/* for now - wants fixing */


#define OFTSIZE		24
#define ITABSIZE	32

#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
