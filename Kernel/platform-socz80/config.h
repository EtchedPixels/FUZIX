/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* CP/M emulation */
#define CONFIG_CPM_EMU
/* We use flexible 16K banks so use the helper */
#define CONFIG_BANK16
#define MAX_MAPS 128

#define CONFIG_BANKS	4	/* For now lets use 16K banking */
#define TICKSPERSEC 100		/* Ticks per second */
#define PROGBASE    0x0000
#define PROGLOAD    0x0100	/* also data base */
#define PROGTOP     0xF800	/* Top of program, base of U_DATA */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We borrow the CP/M command line */
#define CMDLINE	0x81

/* Device parameters */
#define NUM_DEV_TTY 2
#define NUM_DEV_SD 28
#define NUM_DEV_RD 4

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define CONFIG_SD
#define MAX_BLKDEV  1	  /* Only the one SPI supported for now */
#define SD_DRIVE_COUNT 1

#define CONFIG_LARGE_IO_DIRECT(x)	1

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"

