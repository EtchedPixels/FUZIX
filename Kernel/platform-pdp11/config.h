/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL

#define CONFIG_MULTI
#define CONFIG_SWAP_ONLY
#define CONFIG_USERMEM_DIRECT
#define CONFIG_BANKS 	1
#define PROC_SIZE	128			/* 64K, 128 * 512 */

#define CONFIG_SPLIT_UDATA
#define UDATA_SIZE	512
#define UDATA_BLKS	1

#define PROGBASE	0x8000UL
#define PROGLOAD	PROGBASE
#define PROGTOP		0xE000UL
#define SWAP_SIZE	(130 + 2)		/* 2 for the udata */
#define SWAPBASE	PROGBASE
#define SWAPTOP		0xE000UL
#define MAX_SWAPS	PTABSIZE		/* Mandatory for swap only */
#define swap_map(x)	((uint8_t *)(x))

#define SWAPDEV		(1)

#define TICKSPERSEC 50   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define plt_copyright()
