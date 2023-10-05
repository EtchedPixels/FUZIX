/*
 *	If your CPU clock is not 9.216MHz as is the default (eg using an older 6Mhz
 *	part) edit kernel.def and change the clock rate
 */

#define CONFIG_CPU_CLK	6144000

/* Define this to select SD card rather than networkign on the SPI port */
#undef CONFIG_WITH_SD

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

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 40U     /* Ticks per second */

/* Each application has a bank from 00000-CFFF (52K)
   D000-EFFF is a mirror of the common space at F000-FFFF */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xD000  /* Top of program, base of U_DATA */
#define KERNTOP     0xF000  /* Kernel has lower 60KB */
#define PROC_SIZE   56      /* Memory needed per process */

#define CONFIG_TD_NUM	4
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_SDCCPIO
#define CONFIG_TINYIDE_8BIT
#ifdef CONFIG_WITH_SD
#define CONFIG_TD_SD
#define TD_SD_NUM	1
#define SD_SPI_CALLTYPE	__z88dk_fastcall
#endif

#define CONFIG_RTC
#define CONFIG_RTC_FULL
#define CONFIG_RTC_EXTENDED
#define CONFIG_RTC_INTERVAL	100
#define CONFIG_RTC_DS1302

#ifndef CONFIG_WITH_SD
#define CONFIG_NET
#define CONFIG_NET_WIZNET
#define CONFIG_NET_W5500
#endif

/* We need a tidier way to do this from the loader */
#define CMDLINE	(0x0081)  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#"

#define CONFIG_DYNAMIC_BUFPOOL /* we expand bufpool to overwrite the _DISCARD segment at boot */
#define NBUFS    4        /* Number of block buffers, keep in line with space reserved in mark4.s */
#define NMOUNTS	 4	  /* Number of mounts at a time */

/* Hardware parameters : internal hardware at 0xC0-0xFF */
#define Z180_IO_BASE       0xC0

#define NUM_DEV_TTY	2
/* UART0 as the console */
#define BOOT_TTY (512 + 1)
#define TTY_INIT_BAUD	B38400

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x69 	/* 52.5K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xD200	/* Swap out udata and program Dxxx alias Fxxx udata */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define swap_map(x)	((uint8_t *)(x))

#define plt_copyright()		/* for now */

