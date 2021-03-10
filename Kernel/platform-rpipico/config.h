/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#define CONFIG_MULTI
/* 32bit with flat memory */
#undef CONFIG_FLAT
/* Pure swap */
#define CONFIG_BANKS 1
/* brk() calls pagemap_realloc() to get more memory. */
#define CONFIG_BRK_CALLS_REALLOC
/* Inlined irq handling */
#define CONFIG_INLINE_IRQ
/* Trim disk blocks when no longer used */
#define CONFIG_TRIM
/* Enable single tasking */
#define CONFIG_SWAP_ONLY
#define CONFIG_SPLIT_UDATA
/* Enable SD card code. */
#define CONFIG_SD
#define SD_DRIVE_COUNT 1
/* Enable dynamic swap. */
#define CONFIG_PLATFORM_SWAPCTL

#define CONFIG_32BIT
#define CONFIG_USERMEM_DIRECT
/* Serial TTY, no VT or font */
#undef CONFIG_VT
#undef CONFIG_FONT8X8

/* Program layout */

#define UDATA_BLKS  3
#define UDATA_SIZE  (UDATA_BLKS << BLKSHIFT)
#define USERMEM (160*1024)
#define PROGSIZE (65536 - UDATA_SIZE)
extern uint8_t progbase[USERMEM];
#define udata (*(struct u_data*)progbase)

#define USERSTACK (4*1024) /* 4kB */

#define CONFIG_CUSTOM_VALADDR
#define PROGBASE ((uaddr_t)&progbase[0])
#define PROGLOAD ((uaddr_t)&progbase[UDATA_SIZE])
#define PROGTOP (PROGLOAD + PROGSIZE)
#define SWAPBASE PROGBASE
#define SWAPTOP (PROGBASE + (uaddr_t)alignup(udata.u_break - PROGBASE, 1<<BLKSHIFT)) /* never swap in/out data above break */
#define SWAP_SIZE   ((PROGSIZE >> BLKSHIFT) + UDATA_BLKS)
#define MAX_SWAPS   (2048*2 / SWAP_SIZE) /* for a 2MB swap partition */

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

#define TICKSPERSEC 200   /* Ticks per second */
/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

#define BOOTDEVICE 0x0000 /* hda */
#define SWAPDEV    (swap_dev) /* dynamic swap */

/* Device parameters */
#define NUM_DEV_TTY 1

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    20       /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define MAX_BLKDEV	4

#define platform_copyright() /* */
#define swap_map(x) ((uint8_t*)(x))

/* Prevent name clashes wish the Pico SDK */

#define MANGLED 1
#include "mangle.h"

// vim: sw=4 ts=4 et

