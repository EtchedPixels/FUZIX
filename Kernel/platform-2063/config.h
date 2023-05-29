/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed bank sizes in RAM */
#define CONFIG_BANK_FIXED
#define MAP_SIZE 0x8000
#define MAX_MAPS 14	/* one bank high, one for kernel */

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* Memory banks */
#define CONFIG_BANKS	1
#define TICKSPERSEC 10      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0x7E00  /* Top of program */
#define KERNEL_TOP  0xF000  /* Expand buffers up to here */

#define PROC_SIZE   32	  /* Memory needed per process */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers */
#define NMOUNTS	 3	  /* Number of mounts at a time */

#define CONFIG_NET
#define CONFIG_NET_NATIVE

/* IDE/CF support */
#define CONFIG_TD_NUM	1
#define CONFIG_TD_SD
#define SD_SPI_CALLTYPE	__z88dk_fastcall

/* Device parameters */
#define NUM_DEV_TTY 2

/* SIOA as console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B9600

#define CONFIG_VT	/* Mirror console onto VDP and provide graphics hooks */
#define CONFIG_FONT6X8
/* Vt definitions */
#define VT_WIDTH	vt_twidth
#define VT_HEIGHT	24
#define VT_RIGHT	vt_tright
#define VT_BOTTOM	23

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

/* Input device for joysticks */

#define CONFIG_INPUT

#define plt_copyright()

