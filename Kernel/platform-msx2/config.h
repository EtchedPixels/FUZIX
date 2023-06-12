/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_VT_MULTI
/* 16K banking so use the helper */
#define CONFIG_BANK16
#define MAX_MAPS 255

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* As reported to user space - 4 banks, 16K page size */
#define CONFIG_BANKS	4

/* reclaim discarded space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

#define CONFIG_FONT6X8

/* Vt definitions */
#define VT_WIDTH	80
#define VT_HEIGHT	24
#define VT_RIGHT	79
#define VT_BOTTOM	23

/* MODE TEXT2 supports up to 16 VT's */
#define MAX_VT          4

#define TICKSPERSEC 60	    /* default value, it will be upated on device_init */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xE000  /* Top of program, U_DATA somewhere above */

#define BOOT_TTY (512 + 1)        /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 4
#define TTYSIZ  128
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    7       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define CONFIG_SD
#define SD_DRIVE_COUNT 2

#define BOOTDEVICENAMES	"hd#,fd"

#define MAX_BLKDEV 4      /* 2 IDE and 2 SD drives */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL	10
//#define CONFIG_RTC_RP5C01_NVRAM

#define plt_copyright()
