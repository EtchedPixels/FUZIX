#define CONFIG_LARGE_IO_DIRECT(x)	1  /* We support direct to user I/O */

#define CONFIG_FDC765
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

/* Swap based one process in RAM */
#define CONFIG_SWAP_ONLY
#define CONFIG_PARENT_FIRST
#define CONFIG_SPLIT_UDATA
#define UDATA_BLKS	1
#define UDATA_SIZE	0x200
#define CONFIG_DYNAMIC_BUFPOOL
#define CONFIG_DYNAMIC_SWAP
#define MAXTICKS	20	/* Has to be high because we are swap only */

#undef CONFIG_KMOD

#define CONFIG_NET
#define CONFIG_NET_WIZNET
#define CONFIG_NET_W5100

/* Custom banking */

/* Banks as reported to user space */
#define CONFIG_BANKS	1

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xC000  /* Top of program, below C000 for simplicity
                               to get going */

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define SWAPBASE 0x0000
#define SWAPTOP  0xC000UL
#define SWAP_SIZE 0x61		/* 48K + udata */
#define MAX_SWAPS	16
#define SWAPDEV  (swap_dev)  /* Device for swapping (dynamic). */

/* We swap by hitting the user map */
#define swap_map(x)		((uint8_t *)(x))

#define CONFIG_TD
#define CONFIG_TD_NUM	2
/* IDE/CF support */
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
#define IDE_IS_8BIT(x)		1
/* SD support */
#define TD_SD_NUM 2
#define CONFIG_TD_SD
/* Emulator for this platform needs bug workarounds */
#define CONFIG_TD_SD_EMUBUG
#define SD_SPI_CALLTYPE	__z88dk_fastcall

#define BOOTDEVICENAMES "hd#,fd#"

#define CONFIG_SMALL
