/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Single tasking - for now while we get it booting */
#define CONFIG_SINGLETASK
#define PTABSIZE 1

/* Simple user copies for now (change when ROM the kernel) */
#define CONFIG_USERMEM_C
#define BANK_KERNEL
#define BANK_PROCESS

/* Pure swap */
#undef CONFIG_SWAP_ONLY
#define CONFIG_BANKS 1
/* Banked Kernel: need to fix GCC first */
#undef CONFIG_BANKED
/* And swapping */
#define SWAPDEV 6	/* FIXME */
#define SWAP_SIZE   0x80 	/* 64K blocks */
/* FIXME */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0x8000	/* uarea so its a round number of sectors */
#define MAX_SWAPS	32

/* Video terminal, not a serial tty */
#undef CONFIG_VT
/* Simple text mode */
#define CONFIG_VT_SIMPLE
/* Vt definitions */
#define VT_BASE		(uint8_t *)0x6000	/* Default video text mode base */
#define VT_WIDTH	32
#define VT_HEIGHT	16
#define VT_RIGHT	31
#define VT_BOTTOM	15
#define VT_INITIAL_LINE	4

extern unsigned char vt_mangle_6847(unsigned char c);
#define VT_MAP_CHAR(x)	vt_mangle_6847(x)

extern int __user_base;

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    ((uint16_t)(size_t)&__user_base)  /* also data base */
#define PROGLOAD    PROGBASE /* also data base */
#define PROGTOP     0xff80  /* Top of program */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	"0"	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define NDEVS    1        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    6       /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define BIGDATA __attribute__ ((section (".bigbss")))

