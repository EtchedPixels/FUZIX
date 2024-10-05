/* config.h     for the RetroBrew/N8VEM Mini-M68k	*/

#ifndef __CONFIG_H
#define __CONFIG_H

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

#define TICKSPERSEC 50   /* was 100 Ticks per second */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 8
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

/* Could be bigger but we need to add hashing first and it's not clearly
   a win with a CF card anyway */
#define NBUFS    16       /* Number of block buffers */
#define NMOUNTS	 8	  /* Number of mounts at a time */

#define MAX_BLKDEV 4

#define CONFIG_IDE
#define CONFIG_PPIDE		/* MFPIC PPIDE */
#define CONFIG_SD
#define SD_DRIVE_COUNT	2

/* On-board DS1302 on MF-PIC board, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_DS1302

#define plt_copyright()

/* Note: select() in the level 2 code will not work on this configuration
   at the moment as select is limited to 16 processes. FIXME - support a
   hash ELKS style for bigger systems where wakeup aliasing is cheaper */

#define PTABSIZE	32
#define UFTSIZE		16
#define OFTSIZE		64
#define ITABSIZE	96

#define BOOTDEVICENAMES "hd#"

/* TODO: read back the actual current baud in the UART in c_init_hardware
 * and set this accordingly */
#define TTY_INIT_BAUD	B9600

/* NEW below this point */

#define MINI_M68K_VSAVE 0		/* don't fool with exceptions */
#define IOBASE		0xFFFF8000	/* allow 14 bit I/O addresses */
#define IOMAP(x)	(IOBASE|(uint16_t)(x))
#define CONFIG_16x50			/* enable 16x50 UARTs */
#define INT_TIMER	8		/* H-timer uses interrypt 8	  */
#define INT_TTY1	12		/* Console UART interrupt MF/PIC  */
#define CONFIG_16x50			/* common UART in 8250..16C750 class */
#define TICKSPERSECL	8		/* NEW  Ticks per second on low counter (Mini-M68k only) */
/* #define INT_TIMERL */		/* DON'T enable the low timer */

#endif /* __CONFIG_H */
