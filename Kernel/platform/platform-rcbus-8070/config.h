/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Acct syscall support */
#undef CONFIG_ACCT
/* Multiple processes in memory at once */
#define CONFIG_MULTI

/*
 *	Currently we are using the 512K/512K RAM card. We could ue the 8085
 *	style MMU card and linear RAM and that might be better
 */
/* 16K reported page size */
#define CONFIG_PAGE_SIZE	16
/* We use flexible 16K banks with a fixed common */
#define CONFIG_BANK16FC
#define CONFIG_BANKS	4	/* 4 banks 16K page size */
#define MAX_MAPS	32

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Arguments the other way around */
#define CONFIG_CALL_R2L

#define TICKSPERSEC 20	    /* Ticks per second */

#define MAPBASE	    0x0000  /* We map from 0 */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100
#define PROGTOP     0xBE00
#define DP_BASE	    0xFFF0    /* 0xFFF0-0xFFFF internal */
#define DP_SIZE	    0x10

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t swap_dev;
#define SWAP_SIZE   0x60 	/* 48K in blocks (prog + udata) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xC000	/* Swap out udata and program */
#define MAX_SWAPS   16	    	/* We will size if from the partition */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP
#define swap_map(x)	((uint8_t *)(0x8000 + ((x) & 0x3FFF )))

#define CONFIG_TD_NUM	2
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_8BIT
#define IDE_IS_8BIT(x)	1

#define BOOT_TTY 513        /* Set this to default device for stdio, stderr */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5        /* Number of block buffers */
#define NMOUNTS	 2	  /* Number of mounts at a time */

#define plt_discard()
#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
