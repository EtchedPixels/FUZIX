/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
/* 32bit with flat memory */
#undef CONFIG_FLAT
/* Pure swap */
#define CONFIG_BANKS 1
/* Inlined irq handling */
#define CONFIG_INLINE_IRQ
/* Trim disk blocks when no longer used */
#define CONFIG_TRIM
/* Enable single tasking */
#define CONFIG_SINGLETASK
#define CONFIG_SWAP_ONLY
#define CONFIG_SPLIT_UDATA
/* Enable SD card code. */
#define CONFIG_SD
#define SD_DRIVE_COUNT 1

#define CONFIG_32BIT
#define CONFIG_USERMEM_DIRECT
/* Serial TTY, no VT or font */
#undef CONFIG_VT
#undef CONFIG_FONT8X8

/* Program layout */

#define PROGSIZE 65536
extern uint8_t progbase[PROGSIZE];

#define CONFIG_CUSTOM_VALADDR
#define PROGBASE ((uaddr_t)&progbase)
#define PROGTOP (PROGBASE + PROGSIZE)
#define SWAPBASE PROGBASE
#define SWAPTOP PROGTOP
#define SWAP_SIZE   (PROGSIZE >> BLKSHIFT)
#define MAX_SWAPS   20 /* for a 2MB swap partition */
#define UDATA_SIZE  1536
#define UDATA_BLKS  3

static inline uaddr_t pagemap_base(void)
{ return PROGBASE; }

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

#define TICKSPERSEC 200   /* Ticks per second */
/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

#define BOOTDEVICE 0x0012 /* hdb2 */
#define SWAPDEV    0x0011 /* hdb1 */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    10       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV	4

#define platform_copyright() /* */
#define swap_map(x) ((uint8_t*)(x))

/* Unused on this port */

static inline void set_cpu_type(void) {}
static inline void map_init(void) {}
static inline void platform_discard(void) {}
static inline void platform_monitor(void) {}
static inline void platform_reboot(void) {}
static inline void program_vectors(uint16_t* pageptr) {}

/* Prevent name clashes wish the Pico SDK */

#define MANGLED 1
#include "mangle.h"

/* vim: sw=4 ts=4 et: */

