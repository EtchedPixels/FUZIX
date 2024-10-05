/*
 *	Settings you want to touch
 */

/* These reflect a stanard ESP32 configuration */
#define CPU_CLOCK 240		/* We switch to the double clock */

/*
 *	Fuzix definitions for the ESP32 set up
 */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* 32bit with flat memory */
#undef CONFIG_FLAT
/* Pure swap */
#define CONFIG_BANKS 1
/* Inlined irq handling */
#define CONFIG_INLINE_IRQ
/* Trim disk blocks when no longer used */
#define CONFIG_TRIM
/* One process in memory at a time multi-tasking. We do our own custom
   implementation so don't define this */
#undef CONFIG_SWAP_ONLY
/* This requires some further work but is far faster */
#undef CONFIG_PARENT_FIRST
/* Don't thrash when running multiple things */
#define MAXTICKS 200
/* Enable SD card code. */
#define CONFIG_SD

#ifdef CONFIG_ESP_DUAL_SD
#define SD_DRIVE_COUNT 2
#endif
#ifdef CONFIG_ESP_W5500
#define CONFIG_NET
#define CONFIG_NET_WIZNET
#define CONFIG_NET_W5500
#define CONFIG_SPI_SHARED
#endif

#ifndef SD_DRIVE_COUNT
#define SD_DRIVE_COUNT 1
#endif

#define CONFIG_32BIT
#define CONFIG_USERMEM_DIRECT
/* Serial TTY, no VT or font */
#undef CONFIG_VT
#undef CONFIG_FONT8X8

/* Program layout */

extern uint8_t _data_base[];
extern uint8_t _code_base[];
extern uint8_t _data_top[];
extern uint8_t _code_top[];

#define CONFIG_CUSTOM_VALADDR
#define CONFIG_UDATA_TEXTTOP
#define DATABASE    ((uaddr_t)&_data_base)
#define CODEBASE    ((uaddr_t)&_code_base)
#define DATATOP     ((uaddr_t)&_data_top)
#define CODETOP     ((uaddr_t)&_code_top)
#define DATALEN     (DATATOP - DATABASE)
#define CODELEN     (CODETOP - CODEBASE)
#define SWAP_SIZE   ((64+33)*2) /* 64 + 31.5 + 1.5 */
#define MAX_SWAPS   (2048*2 / SWAP_SIZE) /* for a 2MB swap partition */
#define UDATA_SIZE  1536
#define UDATA_BLKS  3

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

#define TICKSPERSEC 200   /* Ticks per second */
/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

#define BOOTDEVICE 0x0011 /* hdb1 */
#define SWAPDEV    (swap_dev) /* wherever */

#define CONFIG_DYNAMIC_SWAP

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#undef CONFIG_DYNAMIC_BUFPOOL
#define NBUFS 4

#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV	4

#define plt_copyright() /* */
#define swap_map(x) ((uint8_t*)(x))


/* vim: sw=4 ts=4 et: */

