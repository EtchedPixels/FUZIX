/* We have an RTC */
#define CONFIG_RTC
/* And we can read ToD from it */
#define CONFIG_RTC_FULL
/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed banking */
#define CONFIG_BANK_FIXED
/* 8 48K banks, 1 is kernel */
#define MAX_MAPS	7
#define MAP_SIZE	0xC000U

/* We are using the RIM/SIM masking for interrupt control not hard di/ei pairs */
#define CONFIG_SOFT_IRQ

/* Read processes and big I/O direct into process space */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 10   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xBE00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   60	  /* Memory needed per process */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */

#define SWAP_SIZE   0x60 	/* 48K in blocks (we actually don't need the low 256) */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC000	/* vectors so its a round number of sectors */
#define MAX_SWAPS   16		/* The full drive would actually be 85! */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP

#define CONFIG_IDE
#define MAX_BLKDEV 1	    /* One IDE */

#define swap_map(x)	((uint8_t *)(x)) /* Simple zero based mapping */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    8	  /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define platform_discard()
#define platform_copyright()

#define BOOTDEVICENAMES "hd#"
