/* Enable to make ^Z dump the inode table for debug */
#define CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL

#define CONFIG_32BIT

#define CONFIG_MULTI
/* This works for banks as kernel is not banked, but won't work when we go
   real MMU */
#define CONFIG_USERMEM_DIRECT
#define CONFIG_BANK_FIXED
#define MAX_MAPS	4
#define MAPBASE		0x00200000
#define MAP_SIZE	0x00100000

#define CONFIG_BANKS 	1
#define PROC_SIZE	MAP_SIZE			/* 1MB */

#define CONFIG_SPLIT_UDATA
#define UDATA_SIZE	1024
#define UDATA_BLKS	2

#define PROGBASE	MAPBASE
#define PROGTOP		(MAPBASE + MAP_SIZE)
#define SWAP_SIZE	((MAP_SIZE/512) + 2)		/* 2 for the udata */
#define SWAPBASE	PROGBASE
#define SWAPTOP		PROGTOP
#define MAX_SWAPS	3				/* So can reuse the same fs */
                                                        /* Use a bigger swap area for real uses ! */
#define swap_map(x)	((uint8_t *)(x))

#define SWAPDEV		(1)

#define TICKSPERSEC 100   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4

