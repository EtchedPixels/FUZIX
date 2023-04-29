/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Pure swap */
#undef CONFIG_SWAP_ONLY

/* 16K reported page size */
#define CONFIG_PAGE_SIZE	48
/* We use flexible 16K banks with a fixed common */
#define CONFIG_BANK16
#define CONFIG_BANKS	4

#define MAX_MAPS	(32-3)

/* And swapping FIXME - needs sorting out yet */
#define SWAPDEV		(swap_dev)	/* Dynamic swap */
#define SWAP_SIZE	0x79		/* 60K in 512 byte blocks plus the udata */
#define SWAPBASE	0x0000
#define SWAPTOP		0xF200		/* User space plus udata block */

#define MAX_SWAPS	32
#define CONFIG_DYNAMIC_SWAP

#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_EXTENDED
#define CONFIG_RTC_DS1302

#define CONFIG_INPUT			/* Input device for joystick */
/*#define CONFIG_INPUT_GRABMAX	3 */

/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. We use the second 16K window.
 */
#define swap_map(x)	((uint8_t *)((((x) & 0x3FFF)) + 0x4000))

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Reclaim the discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

#define MAX_BLKDEV  	2	/* 2 IDE drives */
#define CONFIG_IDE              /* enable if IDE interface present */
#define CONFIG_SD
#define SD_DRIVE_COUNT	1

#define TICKSPERSEC	10   /* Ticks per second */

#define PROGBASE	0x0000  /* also data base */
#define PROGLOAD	0x0100  /* also data base */
#define PROGTOP		0xF000  /* Top of program */

#define DP_BASE		0x0000
#define DP_SIZE		0x0100

#define TTY_INIT_BAUD	B38400
#define BOOT_TTY	(512 + 1)   /* Set this to default device for stdio, stderr */
                              /* In this case, the default is the first TTY device */
                              /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY	2
#define TTYDEV		BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS		5       /* Number of block buffers at boot time */
#define NMOUNTS		2	/* Number of mounts at a time */

extern void plt_discard(void);

#define plt_copyright()		/* for now */

#define BOOTDEVICENAMES	"hd#"
