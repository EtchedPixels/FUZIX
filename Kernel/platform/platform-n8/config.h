/* Allow for 3 disk devices */
#define CONFIG_TD_NUM		3

/* On-board SD on the N8 */
#define CONFIG_TD_SD
#define TD_SD_NUM	1

/* PPIDE support */
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_INDIRECT
#define CONFIG_TINYIDE_PPI

#define CONFIG_FLOPPY

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed banking: 8 x 64K banks, top 4KB is shared with kernel, 60KB-62KB is user memory  */
#define CONFIG_BANK_SPLIT
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* 8 60K banks, 1 is kernel */
#define MAX_MAPS	15
#define MAP_SPLIT	PROGTOP

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 40U     /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF600  /* Top of program, base of U_DATA copy */
#define KERNTOP     0xF000  /* Kernel has lower 60KB */
#define PROC_SIZE   64      /* Memory needed per process */

#define BOOTDEVICENAMES "hd#,,,rd"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in mark4.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x7D 	/* 62.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF800	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define swap_map(x)	((uint8_t *)(x))

/* Hardware parameters */
#define Z180_IO_BASE       0x40

#define MAX_BLKDEV 3	    /* 1 SD drive and 2 IDE for now */

/* On-board SD on the N8 */
#define CONFIG_SD
#define SD_DRIVE_COUNT 1

/* Select IDE disk support, and PPIDE (parallel port IDE) as the interface */
#define CONFIG_IDE
#define CONFIG_PPIDE	/* PPIDE is present */

/* On-board DS1302 on Mark IV, we can read the time of day from it */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

#define CONFIG_INPUT			/* Input device for keyboard */
#define CONFIG_INPUT_GRABMAX	3

#define NUM_DEV_TTY 6

/* Video terminal, not just a serial tty */
#define CONFIG_VT
/* Multiple consoles */
#define CONFIG_VT_MULTI
/* Vt definitions */
#define VT_WIDTH	vt_twidth
#define VT_HEIGHT	24
#define VT_RIGHT	vt_tright
#define VT_BOTTOM	23
#define MAX_VT		4		/* Always come up as lowest minors */

/* Font for the TMS9918A */
#define CONFIG_FONT6X8

/* TMS9918A as the console */
#define TTYDEV   (512+1)  /* System console (used by kernel, init) */
#define TTY_INIT_BAUD B38400

#define plt_copyright()

#define CMDLINE NULL
