/* We have an RTC */
#undef CONFIG_RTC
/* And we can read ToD from it */
#undef CONFIG_RTC_FULL
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
/* 8x 56K banks, 1 is kernel */
#define MAX_MAPS	7
#define MAP_SIZE	0xE000U

/* Networking (not usable yet but for debug/development) */
#undef CONFIG_NET
#undef CONFIG_NET_NATIVE
/* Read processes and big I/O direct into process space */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xDE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   56	  /* Memory needed per process */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

#define CMDLINE	NULL

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5	  /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

//#define BOOTDEVICENNAMES "hd#,fd#,,rd#"
#define BOOTDEVICE (0<<8) /* hda */
