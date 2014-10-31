/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking - for now while we get it booting */
#undef CONFIG_SINGLETASK
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* We use flexible 16K banks so use the helper */
#define CONFIG_BANK16
#define MAX_MAPS 16
/* We want the 4x6 font */
#define CONFIG_FONT_4X6

/* As reported to user space - 4 banks, 16K page size */
#define CONFIG_BANKS	4

/* Vt definitions */
#define VT_WIDTH	120
#define VT_HEIGHT	10
#define VT_RIGHT	119
#define VT_BOTTOM	9

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    ((char *)(0x0100))  /* also data base */
#define PROGTOP     ((char *)(0xF000))  /* Top of program, base of U_DATA */
//#define PROC_SIZE   64	  /* Memory needed per process (for now) */

#define BOOT_TTY (512+1)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3
#define NDEVS    1        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 1	  /* Number of mounts at a time - nothing mountable! */
