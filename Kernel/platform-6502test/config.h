/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Acct syscall support */
#define CONFIG_ACCT
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking - for now while we get it booting */
#undef CONFIG_SINGLETASK
/* Use the C language usermem helpers */
#define CONFIG_USERMEM_C
/* TODO: these need to be defined as the code to flip the banks over */
#define BANK_PROCESS
#define BANK_KERNEL
#define CONFIG_BANKS	1

/* We use flexible 16K banks so use the helper */
#define CONFIG_BANK16
#define MAX_MAPS 16
/* And swapping */
#define SWAPDEV 6	/* FIXME */
#define SWAP_SIZE   0x80 	/* 64K blocks */
/* FIXME */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xF000	/* vectors so its a round number of sectors */
#define UDATA_BLOCKS	0	/* We swap the uarea in the data */
#define UDATA_SWAPSIZE	0
#define MAX_SWAPS	32


/* Video terminal, not a serial tty */
#define CONFIG_VT
/* We want the 8x8 font */
#define CONFIG_FONT_8X8
/* Vt definitions */
#define VT_WIDTH	64
#define VT_HEIGHT	24
#define VT_RIGHT	63
#define VT_BOTTOM	23

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x0200  /* also data base */
#define PROGLOAD    0x0200
#define PROGTOP     0xF000  /* Top of program, base of U_DATA */

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3
#define NDEVS    1        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 1	  /* Number of mounts at a time - nothing mountable! */
