/* System level configuration */

/* Set this if you have the RC2014 CF adapter at 0x10/0x90 */
#define CONFIG_RC2014_CF
/* Set this to be able to do networking */
#define CONFIG_RC2014_NET
/* Set one of these according to the card type you have */
/*#define CONFIG_RC2014_NET_W5100 */	/* W5100 module carrier */
#define CONFIG_RC2014_NET_W5300 /* W5300 module carrier */
/* TODO: bitbang SPI W5x00 */
/* Set this if you have the 8255 IDE adapter */
#define CONFIG_RC2014_PPIDE
/* Set this if you have the floppy interface */
#define CONFIG_RC2014_FLOPPY
/* Set this for SD card support via PIO or SC129 at 0x68 */
#define CONFIG_RC2014_SD
/* Do not set this unless you have the propellor graphics card installed
   and with non TMS9918A firmware as it can't be probed so will be assumed */
#undef CONFIG_RC2014_PROPGFX
/* Below is a work in progress */
/* Set this if using a bus extender. This then builds a special kernel
   for big systems that has some cards on a bus extender. See README. Do not
   set for a "normal" RC2014 system.
 */
#undef CONFIG_RC2014_EXTREME


#define OFTSIZE		56
#define ITABSIZE	40
#define PTABSIZE	20

/*
 *	Turn selections into system level defines
 */

#ifdef CONFIG_RC2014_CF
#define CONFIG_IDE
#endif
#ifdef CONFIG_RC2014_PPIDE
#define CONFIG_IDE
#define CONFIG_PPIDE
#endif
#ifdef CONFIG_RC2014_NET
/* Core Networking support */
#define CONFIG_NET
#ifdef CONFIG_RC2014_NET_W5100
#define CONFIG_NET_WIZNET
#define CONFIG_NET_W5100
#endif
#ifdef CONFIG_RC2014_NET_W5300
#define CONFIG_NET_WIZNET
#define CONFIG_NET_W5300
#endif
#endif
#ifdef CONFIG_RC2014_FLOPPY
#define CONFIG_FLOPPY
#endif
#ifdef CONFIG_RC2014_SD
#define CONFIG_SD
#define SD_DRIVE_COUNT 1
#define SD_SPI_CALLTYPE __z88dk_fastcall
#endif

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* 32 x 16K pages, 3 pages for kernel, whatever the RAM disk uses */
#define MAX_MAPS	(32 - 3)

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 10      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xEE00  /* Top of program, base of U_DATA copy */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x78 	/* 60K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF000	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
/* Kept in bank 2 */
#define CONFIG_DYNAMIC_BUFPOOL
/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. We use the second 16K window
 */
#define swap_map(x)	((uint8_t *)((((x) & 0x3FFF)) + 0x4000))

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd,,rd"

#define NBUFS    5        /* Number of block buffers - must match kernel.def */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 5	    /* 1 floppy, 4 IDE or SD and maybe a ZIP */

#define CONFIG_BLK_PPA

#undef CONFIG_RTC_DS12885
#ifdef CONFIG_RTC_DS12885
#define RTC_ADDR	0xC0B8	/* register address */
#define RTC_DATA	0xC1B8	/* register data */
#define CONFIG_RTC_INTERVAL	10
#endif

/* Enable one RTC interface */
#define CONFIG_RTC_DS1302	/* Standard RC2014 bitbang clock card */
#ifdef CONFIG_RTC_DS1302	/* also used on various single board setups */
#define CONFIG_RTC_INTERVAL	100
#endif

#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_EXTENDED
#define CONFIG_NO_CLOCK

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	3

/* We have a GPIO interface */
#define CONFIG_DEV_GPIO

/* We have I2C */
#define CONFIG_DEV_I2C

/* Video terminal, not just a serial tty */
#define CONFIG_VT
/* Multiple consoles */
#define CONFIG_VT_MULTI
/* Vt definitions */
#define VT_WIDTH	vt_twidth
#define VT_HEIGHT	vt_theight
#define VT_RIGHT	vt_tright
#define VT_BOTTOM	vt_tbottom
#define MAX_VT		4		/* Always come up as lowest minors */

/* We need this for the soft ZX81 support */
#define CONFIG_PLATFORM_UDMA
/* Keyboard contains non-ascii symbols */
#define CONFIG_UNIKEY
/* Font for the TMS9918A and PropGfx */
#define CONFIG_FONT6X8
/* Indirect queues so we can have lots of tty devices */
#define CONFIG_INDIRECT_QUEUES
/* And as they are banked we can make them full Unix size */
#define TTYSIZE		256
typedef uint8_t *queueptr_t;

#define GETQ(x)		qread((x))
#define PUTQ(x,y)	qwrite((x),(y))

extern uint8_t qread(uint8_t *addr) __z88dk_fastcall;
extern void qwrite(uint8_t *addr, uint8_t val);

#define NUM_DEV_TTY 8

/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define Z180_IO_BASE	0xC0

/* AMD FPU */
#define CONFIG_FPU
#define CONFIG_FPU_AMD9511
#define AMD_DATA	0x42
#define AMD_CTL		0x43

#define plt_copyright()		// for now
