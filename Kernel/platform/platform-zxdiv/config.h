#define CONFIG_LARGE_IO_DIRECT(x)	1  /* We support direct to user I/O */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* CP/M emulation */
#undef CONFIG_CPM_EMU

/* Input layer support */
#define CONFIG_INPUT
#define CONFIG_INPUT_GRABMAX	3
/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Keyboard contains non-ascii symbols */
#define CONFIG_UNIKEY
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL

#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_DYNAMIC_SWAP

#define CONFIG_SMALL

/* Custom banking */

/* We have two mappings from our 128K of memory */
#define MAX_MAPS	2
#define MAP_SIZE	0x8000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x8000  /* also data base */
#define PROGLOAD    0x8000  /* also data base */
#define PROGTOP     0xFE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   32	  /* Memory needed per process */
#define MAXTICKS    10	  /* As our task switch is so expensive */

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
#define MAX_BLKDEV 4	    /* 2 IDE drives, 2 SD drive */

#define SWAPBASE 0x8000
#define SWAPTOP  0x10000UL
#define SWAP_SIZE 0x40
#define MAX_SWAPS	16
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */

/* All our pages get mapped into the top 16K bank for swapping use */
#define swap_map(x)		((uint8_t *)(x|0xC000))

#define CONFIG_TD
#define CONFIG_TD_NUM	2
/* IDE/CF support */
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
#define IDE_IS_8BIT(x)	1
/* SD support */
#define TD_SD_NUM 2
#define CONFIG_TD_SD
/* Emulator for this platform needs bug workarounds */
#define CONFIG_TD_SD_EMUBUG
/* Banked so no z88dk fastcalls */
#define SD_SPI_CALLTYPE
#define SD_SPI_BANKED

#define BOOTDEVICENAMES "hd#"
