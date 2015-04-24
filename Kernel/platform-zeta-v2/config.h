/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* Single tasking */
#undef CONFIG_SINGLETASK
/* CP/M emulation */
#undef CONFIG_CPM_EMU
/* Flexible 4x16K banking */
#define CONFIG_BANK16
/* Permit large I/O requests to bypass cache and go direct to userspace */
#define CONFIG_LARGE_IO_DIRECT
/* 32 x 16K pages, 3 pages for kernel, 16 pages for RAM disk */
#define MAX_MAPS	13

/* Banks as reported to user space */
#define CONFIG_BANKS	4

#define TICKSPERSEC 15      /* Ticks per second */
#define PROGBASE    0x0000  /* also data base */
#define PROGLOAD    0x0100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   64	  /* Memory needed per process */

/* WRS: this is probably wrong -- we want to swap the full 64K minus the common code */
/* For now let's just use something and fix this up later when we have a swap device */
#define SWAP_SIZE   0x7F 	/* 63.5K in blocks (which is the wrong number) */
#define SWAPBASE    0x0000	/* start at the base of user mem */
#define SWAPTOP	    0xFF00	/* can we stop at the top? not sure how. let's stop short. */
#define MAX_SWAPS	10	    /* Well, that depends really, hmmmmmm. Pick a number, any number. */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL  /* Location of root dev name */
#define BOOTDEVICENAMES "hd#,fd,,rd"

//#define SWAPDEV  (256 + 1)  /* Device for swapping */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV 4	    /* 1 ROM disk, 1 RAM disk, 1 floppy, 1 SD card */

/* On-board DS1302, we can read the time of day from it */
#define CONFIG_RTC
#define CONFIG_RTC_INTERVAL 30 /* deciseconds between reading RTC seconds counter */

/* Floppy support */
#define CONFIG_FLOPPY		/* #define CONFIG_FLOPPY to enable floppy */

/* PPIDE support */
#define CONFIG_PPIDE 		/* #define CONFIG_PPIDE to enable IDE on 8255A */
#ifdef CONFIG_PPIDE
#define PPIDE_BASE 0x60         /* Base address of 8255A */
#define IDE_REG_INDIRECT        /* IDE registers are not directly connected to the CPU bus */

/* IDE control signal to 8255 port C mapping */
#define PPIDE_A0_LINE           0x01 // Direct from 8255 to IDE interface
#define PPIDE_A1_LINE           0x02 // Direct from 8255 to IDE interface
#define PPIDE_A2_LINE           0x04 // Direct from 8255 to IDE interface
#define PPIDE_CS0_LINE          0x08 // Inverter between 8255 and IDE interface
#define PPIDE_CS1_LINE          0x10 // Inverter between 8255 and IDE interface
#define PPIDE_WR_LINE           0x20 // Inverter between 8255 and IDE interface
#define PPIDE_WR_BIT            5    // (1 << PPIDE_WR_BIT) = PPIDE_WR_LINE
#define PPIDE_RD_LINE           0x40 // Inverter between 8255 and IDE interface
#define PPIDE_RD_BIT            6    // (1 << PPIDE_RD_BIT) = PPIDE_RD_LINE
#define PPIDE_RST_LINE          0x80 // Inverter between 8255 and IDE interface

/* 8255 configuration */
#define PPIDE_PPI_BUS_READ      0x92
#define PPIDE_PPI_BUS_WRITE     0x80

/* IDE register addresses */
#define ide_reg_data      (PPIDE_CS0_LINE)
#define ide_reg_error     (PPIDE_CS0_LINE | PPIDE_A0_LINE)
#define ide_reg_features  (PPIDE_CS0_LINE | PPIDE_A0_LINE)
#define ide_reg_sec_count (PPIDE_CS0_LINE | PPIDE_A1_LINE)
#define ide_reg_lba_0     (PPIDE_CS0_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define ide_reg_lba_1     (PPIDE_CS0_LINE | PPIDE_A2_LINE)
#define ide_reg_lba_2     (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A0_LINE)
#define ide_reg_lba_3     (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define ide_reg_devhead   (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define ide_reg_command   (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define ide_reg_status    (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define ide_reg_altstatus (PPIDE_CS1_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define ide_reg_control   (PPIDE_CS1_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#endif /* CONFIG_PPIDE */

/* Optional ParPortProp board connected to PPI */
//#define CONFIG_PPP		/* #define CONFIG_PPP to enable as tty3 */

/* Device parameters */
#define CONFIG_RAMDISK          /* enable memory-backed disk driver */
#define NUM_DEV_RD 1
#define DEV_RD_PAGES 16		/* size of the RAM disk in pages */
#define DEV_RD_START 48		/* first page used by the RAM disk */

#ifdef CONFIG_PPP
	/* SD card in ParPortProp */
	#define DEVICE_SD
        #define SD_DRIVE_COUNT 1
	#define NUM_DEV_TTY 2

	/* ParPortProp as the console */
	#define BOOT_TTY (512 + 2)
#else
	#define NUM_DEV_TTY 1

	/* UART0 as the console */
	#define BOOT_TTY (512 + 1)
	#define TTY_INIT_BAUD B115200
#endif

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
