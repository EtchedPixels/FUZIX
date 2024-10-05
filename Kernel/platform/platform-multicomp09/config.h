/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Use C helpers for usermem */
#undef CONFIG_USERMEM_C

/* Do larhge I/O direct to the user memory */
#define CONFIG_LARGE_IO_DIRECT(x)	1
/* Reclaim discard space for buffers */
#define CONFIG_DYNAMIC_BUFPOOL

/* We use flexible 16K banks so use the helper */
#define CONFIG_BANK16
#define CONFIG_BANKS	4
/* 512Kbyte RAM in 16K chunks is 32. 3 reserved?
   Since we *could* have 1MByte this should be larger.. 64-3
*/
#define MAX_MAPS 32-3
#define MAPBASE 0x0000
/* And swapping */
/* #define SWAPDEV  2051	 */
#define SWAP_SIZE   0x62
/* FIXME */
#define SWAPBASE    0x0000	/* We swap the lot in one, include the */
#define SWAPTOP	    0xC300	/* uarea so its a round number of sectors */
#define UDATA_BLOCKS	0	/* We swap the uarea in the data */
#define UDATA_SWAPSIZE	0
#define MAX_SWAPS	32
#define swap_map(x)  ((uint8_t *)(x & 0x3fff ))

#undef  CONFIG_LEGACY_EXEC


#define TICKSPERSEC 50      /* Ticks per second */
#define PROGBASE    0x0100  /* also data base */
#define PROGTOP     0xe000  /* Top of program, base of U_DATA */
#define PROGLOAD    0x0100  /* ??? */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* Boot devices */
#define BOOTDEVICENAMES "hd#,,,,,,,,dw"

/* This must be a 16-bit number, not a string! See start.c for examples/encoding
   -- so 0x0001 is hda1
   Without this defined, get prompted for root device at boot time
*/
#define BOOTDEVICE 0x0001

/* We need a tidier way to do this from the loader */
#define CMDLINE	 NULL	  /* Location of root dev name */

/* Allow MBR to be other than at block 0. If so, the start LBA of partitions
   defined in the MBR are defined relative to the position of the MBR, not
   relative to the start of the disk (ie, the values they'd have if the
   MBR was in block 0). Even with this defined, it only acts as a back-up:
   block 0 is still checked first.
*/
#define CONFIG_MBR_OFFSET (0x30000)

/* Device parameters */
#define NUM_DEV_TTY 11
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    6        /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time - nothing mountable! */


/* Drivewire Defines */

#define DW_VSER_NUM 4     /* No of Virtual Serial Ports */
#define DW_VWIN_NUM 4     /* No of Virtual Window Ports */
#define DW_MIN_OFF  3     /* Minor number offset */

/* Block device define. Each block device can have upto 16 partitions */
#define MAX_BLKDEV  1     /* 1 SD drive */

#undef CONFIG_IDE         /* enable if IDE interface present */

#define CONFIG_SD         /* enable if SD  interface present */
#define SD_DRIVE_COUNT 1  /* 1 drive */

#define plt_copyright()	/* For now */
