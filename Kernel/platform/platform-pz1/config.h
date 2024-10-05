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
#define CONFIG_LARGE_IO_DIRECT(x)	(1)
/* Runtime stacks arguments backwards */
#define CONFIG_CALL_R2L

/*
 *	512K RAM (swap yet to do ) - not yet using it all
 *	Question: is common better top or bottom ?
 *	Top means we switch ZP and 6502 stacks, bottom means we don't but
 *	have to copy stuff/watching sharing
 */
#define CONFIG_BANK_FIXED
#define MAX_MAPS	7	/* 9 x 64K */
#define MAP_SIZE	0xDE00

#define TICKSPERSEC	100	/* Ticks per second */

/* We've not yet made the rest of the code - eg tricks match this ! */
#define MAPBASE		0x0000	/* We map from 0 */
#define PROGBASE	0x2000	/* also data base */
#define PROGLOAD	0x2000
#define PROGTOP		0xFE00

/* TODO: swap */
//#define SWAPDEV		1
//#define SWAP_SIZE	127	/* 0xfe00 / 512 */
//#define SWAPBASE	0x0000	/* We swap the lot in one, include the */
//#define SWAPTOP		0xfe00	/* uarea so its a round number of sectors */
//#define UDATA_BLOCKS	0	/* We swap the uarea in the data */
//#define UDATA_SWAPSIZE	0
//#define MAX_SWAPS	16
//#define swap_map(x)	((uint8_t *)(x & 0x3fff ))

#define BOOT_TTY	513	/* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL		/* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY	2
#define TTYDEV		BOOT_TTY/* Device used by kernel for messages, panics */
#define NBUFS		5	/* Number of block buffers */

/* Block device define */
#define CONFIG_TD_NUM	1

#define plt_discard()
#define plt_copyright()

#define MAX_BLKDEV	1
#define NMOUNTS		2	/* Number of mounts at a time */
#define BOOTDEVICENAMES	"hda"

#define CONFIG_SMALL
