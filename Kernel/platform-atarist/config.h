/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* 32bit with flat memory */
#define CONFIG_FLAT
#define CONFIG_SPLIT_ID
#define CONFIG_32BIT
#define CONFIG_BANKS	(65536/512)
#define CONFIG_USERMEM_DIRECT
/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_FONT8X8			/* 8bit font needed */
/* Vt definitions */
/* Hard coded for now */
#define VT_WIDTH	80
#define VT_HEIGHT	50
#define VT_RIGHT	79
#define VT_BOTTOM	49
#define VT_INITIAL_LINE	0

#define TICKSPERSEC 200   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

#define BOOTDEVICENAMES "hd#,fd#"

/* Device parameters */
#define NUM_DEV_TTY 3

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define CONFIG_IDE

#define MAX_BLKDEV	4

/* TODO tty scan rows/cols etc */
