/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL

#define CONFIG_32BIT
#define CONFIG_LEVEL_2

#define CONFIG_MULTI
#define CONFIG_FLAT
#define CONFIG_SPLIT_ID
#define CONFIG_PARENT_FIRST
/* It's not that meaningful but we currently chunk to 512 bytes */
#define CONFIG_BANKS 	(65536/512)

#define CONFIG_LARGE_IO_DIRECT(x)	1

#define CONFIG_SPLIT_UDATA
#define UDATA_SIZE	1024
#define UDATA_BLKS	2

#define TICKSPERSEC 100   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define TTYDEV BOOT_TTY
#define NUM_DEV_TTY 8

/* Video terminal, not just a serial tty */
#define CONFIG_VT
/* Multiple consoles */
#define CONFIG_VT_MULTI
/* Vt definitions */
#define VT_WIDTH	40
#define VT_HEIGHT	24
#define VT_RIGHT	39
#define VT_BOTTOM	23
/* Keyboard contains non-ascii symbols */
#define CONFIG_UNIKEY
/* Font for the TMS9918A */
#define CONFIG_FONT6X8

/* Could be bigger but we need to add hashing first and it's not clearly
   a win with a CF card anyway */
#define NBUFS    16       /* Number of block buffers */
#define NMOUNTS	 8	  /* Number of mounts at a time */

#define MAX_BLKDEV	5

/* RCBUS rtc, we can read the time of day from it */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_EXTENDED
#define CONFIG_RTC_INTERVAL	100

#define CONFIG_IDE
#define CONFIG_PPIDE

#define CONFIG_SD
#define SD_DRIVE_COUNT	3

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	3

#define CONFIG_DEV_GPIO

#define plt_copyright()

#define PTABSIZE	16
#define UFTSIZE		16
#define OFTSIZE		40
#define ITABSIZE	50

#define BOOTDEVICENAMES "hd#"

#define TTY_INIT_BAUD	B38400

/* Our I/O window is at 64K for 64K. */

#define IOMAP(x)	(0x10000+((uint16_t)(x)))

/* Networking */
#define CONFIG_NET
#define CONFIG_NET_NATIVE
