#ifndef __KDATA_DOT_H__
#define __KDATA_DOT_H__

#include <stdbool.h>

extern char *cmdline;

extern struct u_block ub;

/* Some platforms define udata for things like register globals */
#ifndef udata
extern struct u_data udata;
#endif

extern uint16_t maxproc;   /* Actual max number of processes */
extern uint16_t ramsize;
extern uint16_t procmem;
extern uint16_t nproc;	   /* Current number of processes */
extern uint8_t nready;	   /* Number of ready processes */
extern uint8_t inswap;	   /* Set when swapping and IRQs are enabled */

extern inoptr root;        /* Address of root dir in inode table */
extern uint16_t root_dev;  /* Device number of root filesystem. */
extern uint16_t swap_dev;  /* Device number used for swap */

extern struct blkbuf bufpool[];
#ifndef CONFIG_DYNAMIC_BUFPOOL
#define bufpool_end (bufpool + NBUFS)	/* Define so its a compile time const */
#else
extern struct blkbuf *bufpool_end;
#endif

extern struct p_tab ptab[PTABSIZE];
extern struct p_tab *ptab_end;
extern struct oft of_tab[OFTSIZE];       /* Open File Table */
extern struct cinode i_tab[ITABSIZE];    /* In-core inode table */
extern struct mount fs_tab[NMOUNTS];	 /* Mount table */

extern ptptr init_process;  /* The process table address of the first process. */

extern uint8_t ticks_this_dsecond;       /* Tick counter for counting off one decisecond */
extern uint8_t ticks_per_dsecond;	 /* Ticks per decisecond at machine interrupt rate rate */
extern uint16_t runticks;  /* Number of ticks current process has been swapped in */

extern time_t tod;      /* Time of day */

extern ticks_t ticks;

extern ptptr alarms;	   /* List of processes with active alarms/timers */

extern uint8_t *swapbase;  /* Used by device driver for swapping */
extern unsigned swapcnt;
extern blkno_t swapblk;

extern uint16_t waitno;   /* Serial number of processes entering wait state */

extern int16_t acct_fh;	  /* acct() filehandle */
extern struct sysinfoblk sysinfo;

// The device driver switch table
typedef int (*dev_read_t)(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
typedef int (*dev_write_t)(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
typedef int (*dev_init_t)(void);
typedef int (*dev_open_t)(uint_fast8_t minor, uint16_t flag);
typedef int (*dev_close_t)(uint_fast8_t minor);
typedef int (*dev_ioctl_t)(uint_fast8_t minor, uarg_t request, char *data); // note: data is in userspace

typedef struct devsw {
    dev_open_t dev_open;  /* The routines for reading, etc */
    dev_close_t dev_close; /* Format: op(minor,blkno,offset,count,buf); */
    dev_read_t dev_read;  /* Offset would be ignored for block devices */
    dev_write_t dev_write; /* Blkno and offset ignored for tty, etc. */
    dev_ioctl_t dev_ioctl; /* Count is rounded to 512 for block devices */
} devsw;

extern struct devsw dev_tab[];

// Load management
struct runload {
	/* exponent is 8.8 fixed point */
	uint8_t exponent;
	uint16_t average;
};

extern struct runload loadavg[];

// the system call dispatch table
#ifdef CONFIG_LEVEL_2
#define FUZIX_SYSCALL_COUNT 80
#else
#define FUZIX_SYSCALL_COUNT 68
#endif

typedef arg_t (*syscall_t)(void);
extern const syscall_t syscall_dispatch[FUZIX_SYSCALL_COUNT];

extern arg_t _nosys(void);

#endif
