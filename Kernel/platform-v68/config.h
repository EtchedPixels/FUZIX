/* Enable to make ^Z dump the inode table for debug */
#define CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Single tasking - for now while we get it booting */
#undef CONFIG_SINGLETASK

#define CONFIG_32BIT

/* Multiple processes in memory at once */
#define CONFIG_MULTI

#define CONFIG_SWAP_ONLY
#define CONFIG_USERMEM_DIRECT
#define CONFIG_BANKS 	1
#define PROC_SIZE	128

#define PROGBASE	0x20000UL
#define PROGTOP		0x30000UL
#define SWAP_SIZE	256
#define SWAPBASE	PROGBASE
#define SWAPTOP		0x30000UL
#define MAX_SWAPS	PTABSIZE
#define swap_map(x)	((uint8_t *)(x))

#define SWAPDEV		(1)

#if 0
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Buddy based MMU */
#define CONFIG_BUDDY_MMU
#define BUDDY_NUMLEVEL	9		/* 1 MByte */
#define BUDDY_BLOCKBITS	12		/* 4K pages */
#define BUDDY_BLOCKSIZE	4096
/* FIXME: these should be dynamic */
#define BUDDY_BASE	((uint8_t *)0x00000)	/* Manage the low 1MB */
#define BUDDY_START	((uint8_t *)0x10000)	/* First usable 64K */
#define BUDDY_TOP	((uint8_t *)0x80000)	/* End at 512K - should be 7FFFF ? */
#define BUDDY_TABLE	BUDDY_START	/* Pull the table from the pool */
#define BUDDY_TREESIZE	511		/* 1MB in 4K pages */

#define CONFIG_BANKS	1		/* FIXME */
#define CONFIG_USERMEM_BUDDY		/* TODO */
/* Programs run under MMU with zero base */
#define PROGBASE	0

#endif

#define TICKSPERSEC 100   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    20       /* Number of block buffers */
#define NMOUNTS	 6	  /* Number of mounts at a time */

#define MAX_BLKDEV 4

