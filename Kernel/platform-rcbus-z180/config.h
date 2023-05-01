/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Fixed banking: 8 x 64K banks, top 4KB is shared with kernel, 60KB-62KB is user memory upper
   bank is switchable separately */
#define CONFIG_BANK_SPLIT
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* 8 60K banks, 1 is kernel */
#define MAX_MAPS	16
#define MAP_SPLIT	PROGTOP

#define MAP_TRANS_8TO16(M)	(((M) & (unsigned char)0xFE) | 0x8000)
#define MAP_TRANS_16TO8(M)	((unsigned char)(M) | (unsigned char)0x01)

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 40U     /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA */
#define KERNTOP     0xF000  /* Kernel has lower 60KB */
#define PROC_SIZE   64      /* Memory needed per process */

#define CONFIG_TD_NUM		4
/* RC2014 style CF IDE */
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
/* SD via CSIO : Needs an additional GPIO pin so not on all boards. */
#define CONFIG_TD_SD

#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_EXTENDED
#define CONFIG_RTC_INTERVAL	100
#define CONFIG_RTC_DS1302

/* Module work is just an experiment */
#undef CONFIG_KMOD

/* We need a tidier way to do this from the loader */
#define CMDLINE	(0x0081)  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in rcbus-z180.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Hardware parameters : internal hardware at 0xC0-0xFF */
#define Z180_IO_BASE       0xC0

#define NUM_DEV_TTY	2
/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD B115200	/* Matches ROMWBW 3 and later */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x7D 	/* 62.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xF200	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define swap_map(x)	((uint8_t *)(x))

#define plt_copyright()		/* for now */

/* WizNET based TCP/IP : currently you'll need to disable other stuff to
  fit the networking */
#define CONFIG_NET
#define CONFIG_NET_WIZNET
#undef CONFIG_NET_W5200		/* WizNET 5200 */
#define CONFIG_NET_W5500	/* WizNET 5500 */

/* I2C device */
#define CONFIG_DEV_I2C
#define CONFIG_I2C_BITBANG
