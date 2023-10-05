/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 7 64K banks, 1 is kernel */
#define MAX_MAPS	6
#define MAP_SIZE	0xF000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 10      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   60	    /* Memory needed per process */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 3

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5	  /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_LARGE_IO_DIRECT(x)	1

#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd"
