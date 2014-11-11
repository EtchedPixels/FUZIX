/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Single tasking - for now while we get it booting */
#undef CONFIG_SINGLETASK
/* Use C helpers for usermem */
#define CONFIG_USERMEM_C
/* Pure swap */
#define CONFIG_SWAP_ONLY
#define CONFIG_BANKS	1
/* FIXME */
#define BANK_PROCESS	*((volatile uint8_t *)0xff91) &= ~1
#define BANK_KERNEL	*((volatile uint8_t *)0xff91) |= 1
/* And swapping */
#define SWAPDEV 6	/* FIXME */
#define SWAP_SIZE   0x80 	/* 64K blocks */
/* FIXME */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* uarea so its a round number of sectors */
#define UDATA_BLOCKS	0	/* We swap the uarea in the data */
#define UDATA_SWAPSIZE	0
#define MAX_SWAPS	32

/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Simple text mode */
#define CONFIG_VT_SIMPLE
/* Vt definitions */
#define VT_BASE		0x0400	/* Default video text mode base */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    ((uint8_t *)(0x8000))  /* also data base */
#define PROGTOP     ((uint8_t *)(0xff00))  /* Top of program */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define NDEVS    2        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 3	  /* Number of mounts at a time */
