#include <stdint.h>

/* Enable to make ^Z dump the inode table for debug */
#undef CONFIG_IDUMP
/* Enable to make ^A drop back into the monitor */
#undef CONFIG_MONITOR
/* Profil syscall support (not yet complete) */
#undef CONFIG_PROFIL
/* Multiple processes in memory at once */
#undef CONFIG_MULTI
#define PTABSIZE 6
#define MAX_SWAPS PTABSIZE

#define CONFIG_USERMEM_DIRECT

/* Simple user copies for now (change when ROM the kernel) */
#define BANK_KERNEL /* */
#define BANK_PROCESS /* */

/* Pure swap */
#define CONFIG_SWAP_ONLY

#define CONFIG_BANKS 1
/* Banked Kernel: need to fix GCC first */
#undef CONFIG_BANKED
/* And swapping */
#define SWAPDEV 0x0002 /* hda2 */

/* Video terminal, not a serial tty */
#undef CONFIG_VT

/* To save space, queues go in high memory. */
#define CONFIG_INDIRECT_QUEUES
typedef uint16_t queueptr_t;
#define GETQ(p) __read_hidata(p)
#define PUTQ(p, v) __write_hidata((p), (v))

extern int __user_base;
extern int __user_top;

extern int __swap_base;
extern int __swap_top;
extern int __swap_size_blocks;

#define TICKSPERSEC 64   /* Ticks per second */
#define PROGBASE    ((uaddr_t)&__user_base)  /* also data base */
#define PROGLOAD    PROGBASE /* also data base */
#define PROGTOP     ((uaddr_t)&__user_top)  /* Top of program */

#define SWAPBASE    ((uaddr_t)&__swap_base)
#define SWAPTOP	    ((uaddr_t)&__swap_top)
#define SWAP_SIZE   ((uint16_t)&__swap_size_blocks)
#define swap_map(x) ((uint8_t*)(x))

#define BOOT_TTY (512 + 1)   /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */
                            /* Temp FIXME set to serial port for debug ease */

/* We need a tidier way to do this from the loader */
#define CMDLINE	"0"	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 1
#define NDEVS    1        /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    4        /* Number of block buffers */
#define NMOUNTS	 1	      /* Number of mounts at a time */
#define UFTSIZE  15       /* Number of user files */
#define OFTSIZE  15       /* Number of open files */

#define SD_DRIVE_COUNT 1
#define MAX_BLKDEV 1

#define BOOTDEVICE 0x0001 /* hda1 */

extern void plt_discard(void);

