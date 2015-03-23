/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* We want the 8x8 font for now (actually we want 6x8) */
#define CONFIG_FONT8X8
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* Fixed banking */
#undef CONFIG_BANK_FIXED
/* Swap only */
#define CONFIG_SWAP_ONLY
/* Simple user copies for now (change when ROM the kernel) */
#define CONFIG_USERMEM_C
#define BANK_KERNEL
#define BANK_PROCESS

/* Banks as reported to user space */
#define CONFIG_BANKS	1

/* FIXME: the OVL timer isn't quite 100/sec and we have an accurate 1Hz
   timer available, so needs some tweaking */
#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x6000  /* also data base */
#define PROGLOAD    0x6000
#define PROGTOP     0xDD00  /* Top of program */

/* FIXME: treat the banks of the ramdisc as memory not swap, also trim
   to 30K as only have 120K of RAMdisc */
#define SWAP_SIZE   0x3C 	/* 30K in blocks (with uarea means 29.25K max app size) */
#define SWAPBASE    (uint8_t *)0x5D00	/* We swap the lot in one, include the */
#define SWAPTOP	    (uint8_t *)0xDD00	/* vectors so its a round number of sectors */
#define MAX_SWAPS	4	/* We have a whopping 120K of RAMDISC! */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  (256 + 0)  /* Device for swapping. (ram drive) */
#define NBUFS    6        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define VT_WIDTH	30
#define VT_HEIGHT	8
#define VT_RIGHT	29
#define VT_BOTTOM	7


#define PFTABSIZE	5	/* All we have room for right now */
