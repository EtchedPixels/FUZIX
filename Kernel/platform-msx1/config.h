/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking - for now while we get it booting */
#undef CONFIG_SINGLETASK
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Fixed banks */
#define CONFIG_BANK_FIXED
#define MAX_MAPS	16
/* For now lets play simple and assume 32K banks as possible with some of the
   MSX1 megafoo devices */
#define MAP_SIZE	32768

/* As reported to user space - 4 banks, 16K page size */
#define CONFIG_BANKS	1

/* Vt definitions */
#define VT_WIDTH	40
#define VT_HEIGHT	24
#define VT_RIGHT	39
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second (actually should be dynamic FIXME) */
#define PROGBASE    ((char *)(0x0100))  /* also data base */
#define PROGTOP     ((char *)(0x7D00))  /* Top of program (uarea stash) */

#define BOOT_TTY (512 + 1)        /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
