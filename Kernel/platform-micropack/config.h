/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* Fixed banking */
#undef CONFIG_BANK_FIXED
/* Swap only */
#define CONFIG_SWAP_ONLY
/* Mini platform */
#define CONFIG_LEVEL_0

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define PTABSIZE    8
#define ITABSIZE    15
#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x0000	/* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0x6D00	/* Top of program, base of U_DATA */
#define PROC_SIZE   32		/* Memory needed per process */

#define SWAP_SIZE   0x30 	/* in blocks (we actually don't need the low 256) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x6000	/* vectors so its a round number of sectors */
#define MAX_SWAPS   PTABSIZE	/* The full drive would actually be 170! */
#define swap_map(x)	((uint8_t *)(x)) /* Simple zero based mapping */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (256 + 1)  /* Device for swapping. (z80pack drive J) */
#define NBUFS    4        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define platform_discard()
#define platform_copyright()
