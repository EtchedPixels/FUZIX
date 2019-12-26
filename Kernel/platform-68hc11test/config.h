/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
#undef CONFIG_USERMEM_C
/* We use big banks so use the helper */
#define CONFIG_BANK_FIXED
#define CONFIG_BANKS	7	/* and one for the kernel */
#define MAX_MAPS 7
#define MAPBASE	    0x200
#define MAP_SIZE    0xEE00
/* And swapping */
#define SWAPDEV 6	/* FIXME */
#define SWAP_SIZE   0x78 	/* Almost a full 64K */
/* FIXME: udata swap separately */
#define SWAPBASE    0x0200	/* We swap the lot in one */
#define SWAPTOP	    0xF000
#define MAX_SWAPS	32

/* Serial tty */
#undef CONFIG_VT

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x0200  /* also data base */
#define PROGLOAD    0x0200  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of I/O */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    7        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define swap_map(x)	((uint8_t *)(x))

#define platform_discard()
