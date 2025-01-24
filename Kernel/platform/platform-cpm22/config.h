/* We have an RTC but not necessarily.. */
#undef CONFIG_RTC
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
/* We allow for 16 banks including kernel. We could in theory have more
   so this might need to be raised */
/* FIXME: we will need to deal with MAP_SIZE dynamically */
#define MAX_MAPS	15
#define MAP_SIZE	(info->common)
/* Level 2 feature set */
#undef CONFIG_LEVEL_2
/* Networking (not usable yet but for debug/development) */
#undef CONFIG_NET
#undef CONFIG_NET_NATIVE
/* Read processes and big I/O direct into process space */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* We grow the buffer pool dynamically */
#define CONFIG_DYNAMIC_BUFPOOL

#define CONFIG_INPUT			/* Input device for joystick */
#define CONFIG_INPUT_GRABMAX	0	/* No keyboard interface (yet) */

/* We may not have a timer tick */
#define CONFIG_NO_CLOCK

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 10	    /* Ticks per second */
#define PROGBASE    0x0300  /* also data base */
#define PROGLOAD    0x0300  /* also data base */
#define PROGTOP     sys_prog_top  /* Top of udata and program */
#define PROC_SIZE   (info->common >> 10) /* Memory needed per process */

#define SWAP_SIZE   (info->common >> 9) 	/* user size in blocks */
#define SWAPBASE    0x0000		/* We swap the lot in one, include the */
#define SWAPTOP	    (info->common)	/* vectors so its a round number of sectors */
#define MAX_SWAPS   16			/* Assuming we can even find a swap device */
#define SWAPDEV     swap_dev  		/* Device for swapping. */
#define swap_map(x) ((uint8_t *)(x)) /* Simple zero based mapping */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters. Just use aux/con default for now. We can in theory use
   all the CP/M redirections */
#define NUM_DEV_TTY 2

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    4	  /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define plt_copyright()

#define BOOTDEVICENAMES "hd#,fd#"

extern uint16_t sys_prog_top;
extern uint16_t swap_dev;

#include <sysmod.h>

#define CONFIG_SMALL
