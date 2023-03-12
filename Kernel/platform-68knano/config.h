/*
 *	Set these top setings according to your board if different
 */

#define CONFIG_DS3234_SQ	/* Timer off SQ. If undefined assumes a
                                   10Hz external clock instead */
#define CONFIG_RTC_DS3234	/* A DS324 is attached */
/* The UART has clock tables for the following clocks:
   7372800, 8000000, 10000000, 12000000. Update devtty.c if
   adding other clocks */
#define CONFIG_SYSCLK	12000000	/* 12MHz */

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

#define TICKSPERSEC 10   /* Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

/* Could be bigger but we need to add hashing first and it's not clearly
   a win with a CF card anyway */
#define NBUFS    16       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 2

#define CONFIG_IDE

#ifdef CONFIG_RTC_DS3234
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL	255		/* Doesn't matter */
#endif

#define plt_copyright()

/* Size for a slightly bigger setup than the little 8bit boxes */
#define PTABSIZE	32
#define OFTSIZE		30
#define ITABSIZE	50
#define UFTSIZE		16

#define BOOTDEVICENAMES "hd#"

/* We default to 38400 baud because it's soemthing all the clock
   configurations can do */
#define TTY_INIT_BAUD	B38400
