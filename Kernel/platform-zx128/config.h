/* Simple IDE interface */
#define CONFIG_IDE
#define IDE_REG_DATA		0xA3
#define IDE_REG_ERROR		0xA7
#define IDE_REG_FEATURES	0xA7
#define IDE_REG_SEC_COUNT	0xAB
#define IDE_REG_LBA_0		0xAF
#define IDE_REG_LBA_1		0xB3
#define IDE_REG_LBA_2		0xB7
#define IDE_REG_LBA_3		0xBB
#define IDE_REG_DEVHEAD		0xBB
#define IDE_REG_STATUS		0xBF
#define IDE_REG_COMMAND		0xBF

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#define CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* CP/M emulation */
#undef CONFIG_CPM_EMU

/* Video terminal, not a serial tty */
#define CONFIG_VT
/* We want the 8x8 font */
#define CONFIG_FONT8X8
#define CONFIG_FONT8X8SMALL

/* We have 1 bank at C000 with 6 possible pages to map.
   Our choise should be CONFIG_BANK_FIXED, but we are. 
   using custom implementation of it */

#undef CONFIG_BANK_FIXED

/* 6 16K banks, 1 is for kernel needs */
#define MAX_MAPS	5
#define MAP_SIZE	0x4000U

/* Banks as reported to user space */
#define CONFIG_BANKS	1

/* Vt definitions */
#define VT_WIDTH	32
#define VT_HEIGHT	24
#define VT_RIGHT	31
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x8000  /* also data base */
#define PROGLOAD    0xC000  /* also data base */
#define PROGTOP     0xFD00  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   16	  /* Memory needed per process */

#define UDATA_BLOCKS	0	/* We swap the stash not the uarea */
#define UDATA_SWAPSIZE	0

#define BOOT_TTY (513)  /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#undef  SWAPDEV           /* Do not use swap */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
