/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* Swap based one process in RAM */
#define CONFIG_SWAP_ONLY
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* One memory bank */
#define CONFIG_BANKS	1
#define TICKSPERSEC 50      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xC000  /* Top of program */
#define KERNTOP	    0xFFFF  /* Grow buffers to 0xF000 */

#define PROC_SIZE   48	  /* Memory needed per process (inc udata) */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x61 	/* 48.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xC000	/* Swap out program */
#define CONFIG_SPLIT_UDATA	/* Adjacent addresses but different bank! */
#define UDATA_BLKS  1		/* One block of udata */
#define UDATA_SIZE  512		/* 512 bytes of udata */
#define MAX_SWAPS   16	    	/* We will size it from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define MAXTICKS    20
#define CONFIG_PARENT_FIRST	/* Required by the Z80 swap only and good */

/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_FONT6X8
/* Vt definitions */
#define VT_WIDTH	vt_twidth
#define VT_HEIGHT	24
#define VT_RIGHT	vt_tright
#define VT_BOTTOM	23

/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. For our one process setup we can identity map. We'll
 *	have to in fact as MSX1 has no flexible mapping.
 */
#define swap_map(x)	((uint8_t *)(x))

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    5        /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define CONFIG_TINYDISK
#define CONFIG_TD_NUM	2	/* Two devices on the sunrise */

/* We don't use the generic IDE support due to the ghastly MSX slot mappings */

/* Device parameters */
#define NUM_DEV_TTY 2

/* VDP as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define plt_copyright()

extern uint8_t direct_io_range(uint16_t dev);
