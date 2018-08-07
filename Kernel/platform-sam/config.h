/* Set if you want RTC support and have an RTC on ports 0xB0-0xBC */
#undef CONFIG_RTC

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

/* Video terminal, not a serial tty */
#define CONFIG_VT
/* Fonts are pre-expanded as we have reasonable amounts of memory and
   need to deal with a 2bit deep bitmap display */
#define CONFIG_FONT_8X8_EXP2

/* Banked memory set up */
/* FIXME */
#define CONFIG_BANK_FIXED
#define MAX_MAPS	16
#define MAP_SIZE	0xFD00

#define CONFIG_BANKS	2	/* 2 x 32K */

/* Vt definitions */
#define VT_WIDTH	80
#define VT_HEIGHT	24
#define VT_RIGHT	79
#define VT_BOTTOM	23

#define TICKSPERSEC 50   /* Ticks per second */
#define PROGBASE    0x0000  /* Base of user  */
#define PROGLOAD    0x0100  /* Load and run here */
#define PROGTOP     0xFD00  /* Top of program, base of U_DATA stash */
#define PROC_SIZE   64 	    /* Memory needed per process */

#define BOOT_TTY (512 + 1)      /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 2
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */

#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */
