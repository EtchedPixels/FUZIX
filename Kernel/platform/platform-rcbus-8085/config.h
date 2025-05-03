/* We have an RTC */
#define CONFIG_RTC_DS1302
#define CONFIG_RTC
/* And we can read ToD from it */
#define CONFIG_RTC_FULL
/* But our RTC is slow so don't read it all the time */
#define CONFIG_RTC_INTERVAL 100
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
/* 8 56K banks, 1 is kernel, 8K common */
#define MAX_MAPS	7
#define MAP_SIZE	0xE000U

/* Read processes and big I/O direct into process space */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Banks as reported to user space */
#define CONFIG_BANKS	1

#define TICKSPERSEC 10   /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xE000  /* Top of program, base of U_DATA */
#define PROC_SIZE   60	  /* Memory needed per process */

#define SWAPDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */

#define SWAP_SIZE   0x71 	/* 56K in blocks plus udata */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xE200	/* vectors so its a round number of sectors */
#define MAX_SWAPS   16		/* The full drive would actually be 85! */
/* Swap will be set up when a suitably labelled partition is seen */
#define CONFIG_DYNAMIC_SWAP

#define CONFIG_VT
/*#define CONFIG_VT_MULTI
#define MAX_VT	4 */
#define VT_HEIGHT	24
#define VT_BOTTOM	23
#define VT_WIDTH	40	/* vt_twidth */
#define VT_RIGHT	39	/* vt_tright */
#define CONFIG_FONT6X8

#define CONFIG_TINYDISK
#define CONFIG_TD_NUM	4
#define CONFIG_TD_IDE
#define CONFIG_TINYIDE_INDIRECT
#define CONFIG_TD_SCSI
#define CONFIG_CH375

#define swap_map(x)	((uint8_t *)(x)) /* Simple zero based mapping */

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
#define TTY_INIT_BAUD B115200	/* Hardwired generally */

#define CMDLINE	NULL	  /* Location of root dev name */

/* Input device support */
#define CONFIG_INPUT			/* Input device for joystick */

/* Device parameters */
#define NUM_DEV_TTY 2	  /* For now we support a 16x50 and an ACIA */

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5	  /* Number of block buffers must match kernel.def */
#define NMOUNTS	 4	  /* Number of mounts at a time */
#define CONFIG_DYNAMIC_BUFPOOL

#define plt_copyright()

#define BOOTDEVICENAMES "hd#"
