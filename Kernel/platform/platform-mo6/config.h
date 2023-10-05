/* FIXME: swapping and 2000-20FF so we can make user start at 2300 */

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* All our kernel data/common is mapped low and always mapped FIXME */
#define CONFIG_USERMEM_DIRECT

#define CONFIG_BANK_FIXED
#define MAX_MAPS	4	/* 2 unless we can pull cartridge magic */
#define MAP_SIZE	0x7C00	/* 32K minus udata */

#define CONFIG_VT
/* Vt definitions */
#define VT_WIDTH	40
#define VT_HEIGHT	24
#define VT_RIGHT	39
#define VT_BOTTOM	23
#define VT_INITIAL_LINE	0

#define CONFIG_INPUT

#define CONFIG_BANKS	1
/* And swapping */
#define SWAPDEV     0x0		/* Uses part of IDE slice 0 */
#define SWAP_SIZE   0x1F	/* 512 byte blocks */
#define SWAPBASE    0x2200	/* We swap the lot, including uarea */
#define SWAPTOP     0xA000	/* so it's a round number of 512 byte sectors */
#define UDATA_SIZE  0x0200	/* one block */
#define MAX_SWAPS   32

#define PTABSIZE    16

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x2400  /* also data base */
#define PROGLOAD    0x2400  /* also data base */
#define PROGTOP     0xA000  /* Top of program */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

#define CONFIG_TD_NUM	2
#define CONFIG_TD_SD

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   513	 /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define swap_map(x)	((uint8_t *)(x))

#define plt_copyright(x)

#define BOOTDEVICENAMES "hd#,fd"
