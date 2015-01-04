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
/* 16K banking so use the helper */
#define CONFIG_BANK16
#define MAX_MAPS 255

/* As reported to user space - 4 banks, 16K page size */
#define CONFIG_BANKS	4

/* Vt definitions */
#define VT_WIDTH	80
#define VT_HEIGHT	24
#define VT_RIGHT	79
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second (actually should be dynamic FIXME) */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xF000  /* Top of program, base of U_DATA */

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

#define DEVICE_SD
#define SD_DRIVE_COUNT 1

#define MAX_BLKDEV 1      /* Single SD drive */
