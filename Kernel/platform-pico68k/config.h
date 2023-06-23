/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL

#define CONFIG_32BIT

#define CONFIG_MULTI
#define CONFIG_FLAT_SMALL
#define CONFIG_SPLIT_ID
#define CONFIG_PARENT_FIRST

#define CONFIG_LARGE_IO_DIRECT(x)	1

#define PAGEDEV     (swap_dev)	/* A variable for dynamic, or a device major/minor */
extern uint16_t page_dev;
#define CONFIG_DYNAMIC_PAGE
#define swap_map(x)	((uint8_t *)(x))
#define CONFIG_PLATFORM_BRK

#define PAGE_SIZE	2048	/* More would be more efficient but we want */
#define PAGE_SHIFT	11	/* to scrape everything together we can */
#define NBANK		40
#define NPAGE		127	/* The max will be fine */

#define CONFIG_BANKS 	(65536/PAGE_SIZE)

#define CONFIG_SPLIT_UDATA
#define UDATA_SIZE	1024
#define UDATA_BLKS	2

#define TICKSPERSEC	200		/* Ticks per second */

#define BOOT_TTY (512 + 1)	/* Set this to default device for stdio, stderr */
			  /* In this case, the default is the first TTY device */
			    /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL		/* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define TTYDEV   BOOT_TTY	/* Device used by kernel for messages, panics */

/* Keep our sizes low as we are very resource constrained so all memory
   is valuable */
#define PTABSIZE 	8
#define NBUFS    	5	/* Number of block buffers */
#define NMOUNTS	 	2	/* Number of mounts at a time */

#define CONFIG_TD_NUM	1
#define CONFIG_TD_SD
#define TD_SD_NUM	1

#define plt_copyright()

#define BOOTDEVICENAMES "hd#"

#define TTY_INIT_BAUD	B38400
