#ifndef __KDATA_DOT_H__
#define __KDATA_DOT_H__

#include <stdbool.h>

extern char *cmdline;
#define BOOTLINE_LEN 6
extern char bootline[BOOTLINE_LEN];

extern struct u_block ub;
extern struct u_data udata;

extern uint16_t maxproc;   /* Actual max number of processes */
extern uint16_t ramsize;
extern uint16_t procmem;
extern uint16_t nproc;	   /* Current number of processes */
extern uint16_t nready;	   /* Number of ready processes */

extern inoptr root;        /* Address of root dir in inode table */
extern uint16_t root_dev;  /* Device number of root filesystem. */

extern struct blkbuf bufpool[NBUFS];
extern struct p_tab ptab[PTABSIZE];
extern struct p_tab *ptab_end;
extern struct oft of_tab[OFTSIZE];       /* Open File Table */
extern struct cinode i_tab[ITABSIZE];    /* In-core inode table */
extern struct mount fs_tab[NMOUNTS];	 /* Mount table */

extern ptptr init_process;  /* The process table address of the first process. */
extern bool inint;     /* flag is set whenever interrupts are being serviced */

extern uint8_t ticks_this_dsecond;       /* Tick counter for counting off one decisecond */
extern uint16_t runticks;  /* Number of ticks current process has been swapped in */

extern time_t tod;      /* Time of day */

extern ticks_t ticks;

extern uint8_t *swapbase;  /* Used by device driver for swapping */
extern unsigned swapcnt;
extern blkno_t swapblk;

extern uint16_t waitno;   /* Serial number of processes entering wait state */

extern int16_t acct_fh;	  /* acct() filehandle */
extern struct sysinfoblk sysinfo;

// The device driver switch table
typedef int (*dev_read_t)(uint8_t minor, uint8_t rawflag, uint8_t flag);
typedef int (*dev_write_t)(uint8_t minor, uint8_t rawflag, uint8_t flag);
typedef int (*dev_init_t)(void);
typedef int (*dev_open_t)(uint8_t minor, uint16_t flag);
typedef int (*dev_close_t)(uint8_t minor);
typedef int (*dev_ioctl_t)(uint8_t minor, uint16_t request, char *data); // note: data is in userspace

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
#define FUZIX_SYSCALL_COUNT 61
typedef int16_t (*syscall_t)(void);
extern const syscall_t syscall_dispatch[FUZIX_SYSCALL_COUNT];

#endif
