/* NC100 or NC200 - your choice */
/*#define CONFIG_NC200 */

/* We have an RTC */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
/* And it's a fast to access RTC */
#define CONFIG_RTC_INTERVAL 10
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
/* We have a key that needs remapping into unicode space */
#define CONFIG_UNIKEY
/* Custom banking 16 maps */
#define MAX_MAPS 16
/* As we  have low resources default all binaries to about 44K so that
   they fit within 3 pages */
#define DEFAULT_TOP	0xB000
/* We want the 6x8 font */
#define CONFIG_FONT6X8
/* We have audio (just about) */
#define CONFIG_AUDIO

/* As reported to user space - 4 banks, 16K page size */
#define CONFIG_BANKS	4

/* VT definitions */
#ifdef CONFIG_NC200
#define VT_WIDTH	80
#define VT_HEIGHT	16
#define VT_RIGHT	79
#define VT_BOTTOM	15
#else
#define VT_WIDTH	80
#define VT_HEIGHT	8
#define VT_RIGHT	79
#define VT_BOTTOM	7
#endif

#define TICKSPERSEC 100   /* Ticks per second */
#define PROGBASE    0x0000 	/* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xF000 	/* Top of program, base of U_DATA */

#define BOOT_TTY (512+1)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#ifdef CONFIG_NC200
#define NMOUNTS	2	  /* Floppy can also be mounted */
#define BOOTDEVICENAMES "hd#,fd#"
#define MAX_BLKDEV 1  /* Single floppy */
#else
#define NMOUNTS	 1	  /* Number of mounts at a time - nothing mountable! */
#define BOOTDEVICE 0x0000	/* Only one possible option */
#endif

#define CONFIG_LARGE_IO_DIRECT(m)	1
			/* Definite win as our I/O is as fast as a memcpy! */
#define plt_discard()
#define plt_copyright()
