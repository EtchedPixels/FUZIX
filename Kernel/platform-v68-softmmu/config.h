/* Enable to make ^Z dump the inode table for debug */
#define CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL

#define CONFIG_32BIT
#define CONFIG_MULTI
#define CONFIG_USERMEM_DIRECT
#define CONFIG_VMMU

#define CONFIG_LARGE_IO_DIRECT(x)	1

#define MMU_BLKSIZE	8192

#define CONFIG_BANKS 	1

#define TICKSPERSEC 100   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4

#define platform_copyright()

#definbe BOOTDEVICENAMES "hd#"
