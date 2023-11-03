/*
 *	Build options
 *
 *	See README.CONFIG
 */

#define CONFIG_TD_IDE		/* GIDE */
#undef CONFIG_TD_SD		/* SD card bitbanged on I/O port */
#define CONFIG_RD_SWAP		/* Swap on the ramdisc not GIDE */
#undef CONFIG_PIO_TICK		/* 10Hz square wave on PIO bit 3 for timer */
#define CONFIG_K1520_SOUND	/* We only use the CTC bits for now */
#define CONFIG_JKCEMU		/* Work around JKCEMU problems
                                    - no LBA emulation
                                    - buggy disk emulation
                                    - probably a bug in port 4 handling */
#undef CONFIG_VIDEO_POPPE	/* 64x32 video. Set this in kernelu.def also */
#undef CONFIG_RTC_70		/* RTC at 0x70 (not GIDE RTC) */
#undef CONFIG_FDC765		/* Floppy controller (not yet done) - also set in kernelu.def */

/*
 *	Platform configuration
 */

#ifdef CONFIG_VIDEO_POPPE
#define MEM_TOP		0xE800
#define PROC_SIZE	34
#define SWAP_SIZE	0x45
#else
#define MEM_TOP		0xEC00
#define PROC_SIZE	35
#define SWAP_SIZE	0x47
#endif

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
#define TICKSPERSEC 10		/* Ticks per second */
#define PROGBASE    0x6000	/* also data base */
#define PROGLOAD    0x6000	/* also data base */
#define PROGTOP     MEM_TOP	/* Top of program */
#define KERNTOP	    0x5FF0	/* Grow buffers up to user space (5FFx is vectors) */

#define SWAPBASE    0x6000	/* start at the base of user mem */
#define SWAPTOP	    MEM_TOP	/* Swap out program */
#define CONFIG_SPLIT_UDATA
#define UDATA_BLKS  1
#define UDATA_SIZE  0x200	/* One block */

#ifdef CONFIG_RD_SWAP
#define UTFSIZE		15
#define OFTSIZE		15
#define ITABSIZE	20
#define MAX_SWAPS   	7
#define PTABSIZE	7
#define SWAPDEV		0x300
#else
#define MAX_SWAPS   16	    	/* We will size if from the partition */
#define CONFIG_SMALL
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#endif

#define MAXTICKS    20		/* As we are pure swap */
#define CONFIG_PARENT_FIRST	/* For pure swap this is far faster */

/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. For a simple banked setup there is no conversion
 *	needed so identity map it.
 */
#define swap_map(x)	((uint8_t *)(x))

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in zeta-v2.s */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define CONFIG_TD_NUM	4

/* On-board RTC on the GIDE or an RTC card */
#if defined(CONFIG_TD_IDE) || defined(CONFIG_RTC_70)
#define CONFIG_TINYIDE_INDIRECT
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL	1
#endif

#ifdef CONFIG_TD_SD
#define TD_SD_NUM	1
#endif

#if !defined(CONFIG_PIO_TICK) && !defined(CONFIG_K1520_SOUND)
#define CONFIG_NO_CLOCK
#endif

/* JKCEMU has some limits */
#ifdef CONFIG_JKCEMU
#define CONFIG_TD_IDE_CHS	/* No LBA support in emulation */
#define TD_IDE_NUM	1	/* Work around buggy emulator */
#endif

/* Device parameters */
#define NUM_DEV_TTY 1		/* Only a console */

/* Console */
#define CONFIG_VT

#ifdef CONFIG_VIDEO_POPPE
#define VT_WIDTH	64
#define VT_HEIGHT	32
#define VT_RIGHT	63
#define VT_BOTTOM	31
#else
/* Vt definition */
#define VT_WIDTH	32
#define VT_HEIGHT	32
#define VT_RIGHT	31
#define VT_BOTTOM	31
#define VT_INITIAL_LINE	3
#endif

/* Video as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define plt_copyright()
