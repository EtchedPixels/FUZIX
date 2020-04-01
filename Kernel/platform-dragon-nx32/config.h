/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Pure swap */
#undef CONFIG_SWAP_ONLY

#define CONFIG_BANK_FIXED
#define MAX_MAPS 16
#define MAP_SIZE 0x7F00U
#define CONFIG_BANKS	1
/* And swapping */
#define SWAPDEV (swap_dev)	/* Dynamic swap */
#define SWAP_SIZE   0x40	/* 32K in 512 byte blocks */
#define SWAPBASE    0x8000	/* We swap the lot, including stashed uarea */
#define SWAPTOP     0xFF00	/* so it's a round number of 256 byte sectors */
#define MAX_SWAPS   32
#define CONFIG_DYNAMIC_SWAP

/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Reclaim the discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

#define MAX_BLKDEV  	3	/* 2 IDE drives + 1 SPI */
#define SD_DRIVE_COUNT	1	/* Could be higher with multiple CS used */
#define CONFIG_SD               /* enable if SD  interface present */
#define CONFIG_IDE              /* enable if IDE interface present */
#define CONFIG_SCSI             /* enable if SCSI interface present */

#define CONFIG_GPT

/* Video terminal, not a serial tty */
#define CONFIG_VT
#define CONFIG_VT_MULTI
#define CONFIG_FONT8X8
/* Vt definitions */
#define VT_RIGHT	(vt_tright[curtty])
#define VT_BOTTOM	(vt_tbottom[curtty])
#define VT_INITIAL_LINE	0
#define MAX_VT 4

#define VIDEO_BASE	0x0400
#define VC_BASE		0x1C00

#define CRT9128_BASE	0xFF7C

#define TICKSPERSEC 50   /* Ticks per second */
/* FIXME: This will move once we put the display in the kernel bank and
   sort the banker out */
#define PROGBASE    0x8000  /* also data base */
#define PROGLOAD    0x8000  /* also data base */
#define PROGTOP     0xFD00  /* Top of program */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 7
#define NDEVS    2        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    5       /* Number of block buffers at boot time */
#define NMOUNTS	 2	  /* Number of mounts at a time */
#define swap_map(x)	((uint8_t *)(x))

/* Drivewire Defines */

#define DW_VSER_NUM 1     /* No of Virtual Serial Ports */
#define DW_VWIN_NUM 1     /* No of Virtual Window Ports */
#define DW_MIN_OFF  6     /* Minor number offset = first DW ttyX */

/* Remember to update platform-dragon-nx32/kernel.defs to match */

extern void platform_discard(void);

#define platform_copyright()		/* for now */

#define BOOTDEVICENAMES "hd#,fd#"
