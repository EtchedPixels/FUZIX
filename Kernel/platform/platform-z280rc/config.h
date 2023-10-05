/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI

/* This is a quick hack before we worry about the real MMU and all the rest
   of it */

/* Select a banked memory set up */
#define CONFIG_BANK_FIXED
/* This is the number of banks of user memory available (maximum) */
#define MAX_MAPS	15		/* 512 KByte... minus the high one */
/* How big is each bank - in our case 32K, 48K is actually more common. This
   is hardware dependant */
#define MAP_SIZE	0xE000
/* How many banks do we have in our address space */
#define CONFIG_BANKS	1

/*
 *	Define the program loading area (needs to match kernel.def)
 */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0xDE00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   48	    /* Memory needed per process including stash */
/*
 *	Definitions for swapping.
 */
#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x70 	/* 56K in 512 byte blocks */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xE000	/* vectors so its a round number of sectors */

#define MAX_SWAPS	16	/* Maximum number of swapped out processes.
                                   As we use the default 15 process max this
                                   is definitely sufficient (14 would do) */
/*
 *	When the kernel swaps something it needs to map the right page into
 *	memory using map_for_swap and then turn the user address into a
 *	physical address. For a simple banked setup there is no conversion
 *	needed so identity map it.
 */
#define swap_map(x)	((uint8_t *)(x))

/* Set these two for networking - no point right now */
//#define CONFIG_NET
//#define CONFIG_NET_NATIVE


/* What is the maximum number of /dev/hd devices we have. In theory right now
   it's actually 3 - two in the IDE and one on the SD interface */
#define MAX_BLKDEV	4
/* Select IDE disk support, and PPIDE (parallel port IDE) as the interface */
#define CONFIG_IDE
#define CONFIG_PPIDE	/* PPIDE is present */

/* We will resize the buffers available after boot. This is the normal setting */
#define CONFIG_DYNAMIC_BUFPOOL
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
/* Larger transfers (including process execution) should go directly not via
   the buffer cache. For all small (eg bit) systems this is the right setting
   as it avoids polluting the small cache with data when it needs to be full
   of directory and inode information */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Specify this if there is a real time clock capable of reporting seconds. It
   will be used to lock the kernel time better to reality. Other details like
   Y2K support, or even supporting dates as well don't matter */
#undef CONFIG_RTC
/* Specify that there is a full real time clock that can supply the date and
   time to the system. */
#undef CONFIG_RTC_FULL
/* Set this if the system has no proper real time clock (or has configurations
   where it lacks one). This is not usually needed but for platforms it is also
   see platform-sbcv2/main.c on what is needed */
#undef CONFIG_NO_CLOCK
/*
 * How fast does the clock tick (if present), or how many times a second do
 * we simulate if not. For a machine without video 10 is a good number. If
 * you have video you probably want whatever vertical sync/blank interrupt
 * rate the machine has. For many systems it's whatever the hardware gives
 * you.
 *
 * Note that this needs to be divisible by 10 and at least 10. If your clock
 * is a bit slower you may need to fudge things somewhat so that the kernel
 * gets 10 timer interrupt calls per second. 
 */
#define TICKSPERSEC 10	    /* Ticks per second */

/*
 *	The device (major/minor) for the console and boot up tty attached to
 *	init at start up. 512 is the major 2, so all the tty devices are
 *	512 + n where n is the tty.
 */
#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
/*
 *	If you have a mechanism to pass in a root device configuration then
 *	this holds the address of the buffer (eg a CP/M command line or similar).
 *	If the configuration is fixed then this can be a string holding the
 *	configuration. NULL means 'prompt the user'.
 */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1	  /* How many tty devices does the platform support */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers. Must be 4+ and must match
                             kernel.def */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* This can optionally be set to force a default baud rate, eg if the system
   console should match a firmware set rate */
#define TTY_INIT_BAUD B38400	/* To match ROMWBW */

#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
