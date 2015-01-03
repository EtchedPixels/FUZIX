/****************************************************
FUZIX (Unix Z80 Implementation) Kernel:  unix.h
From UZI by Doug Braun and UZI280 by Stefan Nitschke.
*****************************************************/
/* History:
 *   21.12.97 - Removed leading ? from Per-Process equates.	HFB
 *   11.07.98 - Shortened Time Slices to 50 Ticks/Sec (20 mS).	HFB
 */

#ifndef __FUZIX__KERNEL_DOT_H__
#define __FUZIX__KERNEL_DOT_H__

#include <stdbool.h>

#include "config.h"
#include "cpu.h"

#ifndef NULL
#define NULL (void *)0
#endif

#define min(a,b) ( (a) < (b) ? (a) : (b) )
#define max(a,b) ( (a) > (b) ? (a) : (b) )

#define CPM_EMULATOR_FILENAME    "/usr/cpm/emulator"

/* Maximum UFTSIZE can be is 16, then you need to alter the O_CLOEXEC code */

#ifndef UFTSIZE
#define UFTSIZE 10       /* Number of user files */		/*280 = 22*/
#endif
#ifndef OFTSIZE
#define OFTSIZE 15       /* Open file table size */		/*280 = 45*/
#endif
#ifndef ITABSIZE
#define ITABSIZE 20      /* Inode table size */			/*280 = 45*/
#endif
#ifndef PTABSIZE
#define PTABSIZE 15      /* Process table size. */
#endif

#define MAXTICKS     10   /* Max ticks before switching out (time slice)
                            default process time slice */
// #define MAXBACK      3   /* Process time slice for tasks not connected
//                             to the current tty */
// #define MAXBACK2     2   /* Process time slice for background tasks */
// #define MAXINTER     5   /* Process time slice for interactive tasks */

#define MAXPID    32000

#define NSIGS	  32      /* Number of signals <= 32 */
#define ROOTINODE 1       /* Inode # of / for all mounted filesystems. */
#define CMAGIC    24721   /* Random number for cinode c_magic */
#define SMOUNTED  12742   /* Magic number to specify mounted filesystem */

#define OS_BANK 0
#define NO_DEVICE (0xFFFFU)
#define NO_FILE   (0xFF)

typedef struct s_queue {
    unsigned char *q_base;    /* Pointer to data */
    unsigned char *q_head;    /* Pointer to addr of next char to read. */
    char *q_tail;    /* Pointer to where next char to insert goes. */
    int   q_size;    /* Max size of queue */
    int   q_count;   /* How many characters presently in queue */
    int   q_wakeup;  /* Threshold for waking up processes waiting on queue */
} queue_t;

struct tms {
	clock_t  tms_utime;
	clock_t  tms_stime;
	clock_t  tms_cutime;
	clock_t  tms_cstime;
	clock_t  tms_etime;      /* Elapsed real time */
} ;

/* Flags for setftime() */
#define A_TIME 1
#define M_TIME 2
#define C_TIME 4

typedef int32_t off_t;	/* 32MB file and fs size limit */

typedef uint16_t blkno_t;    /* Can have 65536 512-byte blocks in filesystem */
#define NULLBLK ((blkno_t)-1)

#define BLKSIZE		512
#define BLKSHIFT	9
#define BLKMASK		511

#define SLICE_SIZE_LOG2_SECTORS 16    /* 32MB slices */

/* FIXME: if we could split the data and the header we could keep blocks
   outside of our kernel data (as ELKS does) which would be a win, but need
   some more care on copies, block indexes and directory ops */
typedef struct blkbuf {
    uint8_t     bf_data[BLKSIZE];    /* This MUST be first ! */
    uint16_t    bf_dev;
    blkno_t     bf_blk;
    bool        bf_dirty;
    bool        bf_busy;
    uint16_t    bf_time;         /* LRU time stamp */
} blkbuf, *bufptr;

/* TODO: consider smaller inodes or clever caching. 2BSD uses small
   direct block lists to keep inodes small as they must be in memory when
   'live' - we could also split them into the 28 bytes we always need to
   keep live (i_addr[0] for the dev ptr) and the 36 we don't (or 32/32 for
   speed). We'd then be able to drop half the bits for an open inode onto
   disk safely */
typedef struct dinode {
    uint16_t i_mode;
    uint16_t i_nlink;
    uint16_t i_uid;
    uint16_t i_gid;
    off_t    i_size;
    uint32_t   i_atime;		/* Breaks in 2038 */
    uint32_t   i_mtime;		/* Need to hide some extra bits ? */
    uint32_t   i_ctime;		/* 24 bytes */
    blkno_t  i_addr[20];
} dinode;               /* Exactly 64 bytes long! */

struct  stat    /* Really only used by libc */
{
	int16_t   st_dev;
	uint16_t  st_ino;
	uint16_t  st_mode;
	uint16_t  st_nlink;
	uint16_t  st_uid;
	uint16_t  st_gid;
	uint16_t  st_rdev;
	off_t   st_size;
	uint32_t  st_atime;	/* Break in 2038 */
	uint32_t  st_mtime;
	uint32_t  st_ctime;
};

/* We use the Linux one for compatibility. There's no real Unix 'standard'
   for such things */

struct hd_geometry {
	uint8_t heads;
	uint8_t sectors;
	uint16_t cylinders;
	uint32_t start;
};
#define HDIO_GETGEO		0x0101
#define HDIO_GET_IDENTITY	0x0102	/* Not yet implemented anywhere */

/* Bit masks for i_mode and st_mode */

#define OTH_EX  0001
#define OTH_WR  0002
#define OTH_RD  0004
#define GRP_EX  0010
#define GRP_WR  0020
#define GRP_RD  0040
#define OWN_EX  0100
#define OWN_WR  0200
#define OWN_RD  0400

#define SAV_TXT 01000
#define SET_GID 02000
#define SET_UID 04000

#define MODE_MASK 07777

#define F_REG   0100000
#define F_DIR   040000
#define F_PIPE  010000
#define F_BDEV  060000  // important that F_BDEV & F_CDEV != 0 (see isdevice() function)
#define F_CDEV  020000
#define F_SOCK	0140000

#define F_MASK  0170000

#define major(x) ((x) >> 8)
#define minor(x) ((x) & 0xFF)

typedef struct cinode { // note: exists in memory *and* on disk
    uint16_t   c_magic;           /* Used to check for corruption. */
    uint16_t   c_dev;             /* Inode's device */
    uint16_t   c_num;             /* Inode # */
    dinode     c_node;
    uint8_t    c_refs;            /* In-core reference count */
    uint8_t    c_flags;           
#define CDIRTY		0x80	/* Modified flag. */
#define CFLOCK		0x0F	/* flock bits */
#define CFLEX		0x0F	/* locked exclusive */
#define CFMAX		0x0E	/* highest shared lock count permitted */
} cinode, *inoptr;

#define NULLINODE ((inoptr)NULL)
#define NULLINOPTR ((inoptr*)NULL)

#define FILENAME_LEN	30
#define DIR_LEN		32
typedef struct direct {
    uint16_t   d_ino;
    char     d_name[FILENAME_LEN];
} direct;


/*
 *	This is actually overlaid over a blkbuf holding the actual
 *	record in question, and pinned until we umount the fs.
 */
#define FILESYS_TABSIZE 50
typedef struct filesys { // note: exists in mem and on disk
    int16_t       s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    uint16_t      s_nfree;
    blkno_t       s_free[FILESYS_TABSIZE];
    int16_t       s_ninode;
    uint16_t      s_inode[FILESYS_TABSIZE];
    bool          s_fmod;
    uint8_t       s_timeh;	/* bits 32-40: FIXME - wire up */
    uint32_t      s_time;
    blkno_t       s_tfree;
    uint16_t      s_tinode;
    inoptr        s_mntpt;     /* Mount point */
} filesys, *fsptr;

typedef struct oft {
    off_t     o_ptr;      /* File position pointer */
    inoptr    o_inode;    /* Pointer into in-core inode table */
    uint8_t   o_access;   /* O_RDONLY, O_WRONLY, or O_RDWR + flag bits */
    uint8_t   o_refs;     /* Reference count: depends on # of active children */
} oft;

/* Mount table entries */
struct mount {
    uint16_t m_dev;
    uint16_t m_flags;
    struct filesys *m_fs;
};
/* The flags are not yet implemented */
#define MS_RDONLY	1
#define MS_NOSUID	2

/* Process table p_status values */

#define P_EMPTY         0    /* Unused slot */
#define P_RUNNING       1    /* Currently running process (must match value in kernel.def) */
/* The sleeping range must be together see swap.c */
#define P_READY         2    /* Runnable   */
#define P_SLEEP         3    /* Sleeping; can be awakened by signal */
#define P_XSLEEP        4    /* Sleeping, don't wake up for signal */
#define P_PAUSE         5    /* Sleeping for pause(); can wakeup for signal */
#define P_WAIT          6    /* Executed a wait() */
#define P_FORKING       7    /* In process of forking; do not mess with */
#define P_ZOMBIE2       8    /* Exited but code pages still valid. */
#define P_ZOMBIE        9    /* Exited. */


/* 0 is used to mean 'check we could signal this process' */

/* FIXME: finish signal handling */
#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO
#define SIGPWR		30
#define SIGSYS		31
#define	SIGUNUSED	31

#define  SIG_DFL   (int16_t (*)())0
#define  SIG_IGN   (int16_t (*)())1

#define sigmask(sig)    (1UL<<(sig))

/* uadmin */

#define A_SHUTDOWN		1
#define A_REBOOT		2
#define A_DUMP			3
#define A_FREEZE		4	/* Unimplemented, want for NC100 */
#define A_SWAPCTL		16	/* Unimplemented */
#define A_CONFIG		17	/* Unimplemented */
#define A_FTRACE		18	/* Unimplemented: 
                                          Hook to the syscall trace debug */

#define AD_NOSYNC		1	/* Unimplemented */
                                          
/* Process table entry */

typedef struct p_tab {
    /* WRS: UPDATE kernel.def IF YOU CHANGE THIS STRUCTURE */
    uint8_t     p_status;       /* Process status: MUST BE FIRST MEMBER OF STRUCT */
    uint8_t     p_tty;          /* Process' controlling tty minor # */
    uint16_t    p_pid;          /* Process ID */
    uint16_t    p_uid;
    struct p_tab *p_pptr;      /* Process parent's table entry */
    uint16_t    p_alarm;        /* Centiseconds until alarm goes off */
    uint16_t    p_exitval;      /* Exit value */
    void *      p_wait;         /* Address of thing waited for */
    uint16_t    p_page;         /* Page mapping data */
    uint16_t	p_page2;	/* It's really four bytes for the platform */
    /* Update kernel.def if you change fields above this comment */
    /* Everything below here is overlaid by time info at exit */
    uint16_t    p_priority;     /* Process priority */
    uint32_t    p_pending;      /* Bitmask of pending signals */
    uint32_t    p_ignored;      /* Bitmask of ignored signals */
    uint32_t    p_held;         /* Bitmask of held signals */
    struct u_block *p_ublk;     /* Pointer to udata block when not running */
    uint16_t    p_waitno;       /* wait #; for finding longest waiting proc */
    uint16_t    p_timeout;      /* timeout in centiseconds - 1 */
                                /* 0 indicates no timeout, 1 = expired */

/**HP**/
    char    p_name[8];
    clock_t p_time, p_utime, p_stime, p_cutime, p_cstime;
/**HP**/
    uint16_t	p_pgrp;		/* Process group */
    uint8_t	p_nice;
#ifdef CONFIG_PROFIL
    uint8_t	p_profscale;
    void *	p_profbuf;
    uint16_t	p_profsize;
    uint16_t	p_profoff;
#endif    
} p_tab, *ptptr;

typedef struct u_data {
    /**** If you change this top section, also update offsets in "kernel.def" ****/
    struct p_tab *u_ptab;       /* Process table pointer */
    uint16_t    u_page;         /* Process page data (equal to u_ptab->p_page) */
    uint16_t    u_page2;        /* Process page data (equal to u_ptab->p_page2) */
    bool        u_insys;        /* True if in kernel */
    uint8_t     u_callno;       /* sys call being executed. */
    void *      u_syscall_sp;   /* Stores SP when process makes system call */
    susize_t    u_retval;       /* Return value from sys call */
    int16_t     u_error;        /* Last error number */
    void *      u_sp;           /* Stores SP when process is switchped */
    bool        u_ininterrupt;  /* True when the interrupt handler is runnign (prevents recursive interrupts) */
    int8_t      u_cursig;       /* Next signal to be dispatched */
    arg_t       u_argn;         /* Last system call arg */
    arg_t       u_argn1;        /* This way because args on stack backwards */
    arg_t       u_argn2;
    arg_t       u_argn3;        /* args n-3, n-2, n-1, and n */
    void *      u_isp;          /* Value of initial sp (argv) */
    usize_t	u_top;		/* Top of memory for this task */
    int16_t     (*u_sigvec[NSIGS])();   /* Array of signal vectors */
    /**** If you change this top section, also update offsets in "kernel.def" ****/

    char *      u_base;         /* Source or dest for I/O */
    usize_t     u_count;        /* Amount for I/O */
    off_t       u_offset;       /* Place in file for I/O */
    struct blkbuf *u_buf;
    bool        u_sysio;        /* True if I/O to system space */

    /* This block gets written to acct */
    
    /* We overwrite u_mask with p->p_uid on the exit */
    uint16_t    u_mask;         /* umask: file creation mode mask */
    uint16_t    u_gid;
    uint16_t    u_euid;
    uint16_t    u_egid;
    uint16_t    u_break;        /* Top of data space */
    char        u_name[8];      /* Name invoked with */
    clock_t     u_utime;        /* Elapsed ticks in user mode */
    clock_t     u_stime;        /* Ticks in system mode */
    clock_t     u_cutime;       /* Total childrens ticks */
    clock_t     u_cstime;
    clock_t     u_time;         /* Start time */
    
    /* This section is not written out except as padding */
    uint8_t     u_files[UFTSIZE];       /* Process file table: indices into open file table, or NO_FILE. */
    uint16_t	u_cloexec;	/* Close on exec flags */
    inoptr      u_cwd;          /* Index into inode table of cwd. */
    inoptr	u_root;		/* Index into inode table of / */
    //inoptr      u_ino;          /* Used during execve() */
    inoptr	u_rename;	/* Used in n_open for rename() checking */
} u_data;


/* This is the user data structure, padded out to 512 bytes with the
 * System Stack.
 */
typedef struct u_block {
        u_data u_d;
        char   u_s [512 - sizeof(struct u_data)];
} u_block;


/* Struct to temporarily hold arguments in execve */
struct s_argblk {
    int a_argc;
    int a_arglen;
    int a_envc;
    uint8_t a_buf[512-3*sizeof(int)];
};


/* waitpid options */
#define WNOHANG		1	/* don't support others yet */


/* Open() parameters. */

/* Bits 0-7 are saved, bits 8-15 are discard post open. Not all are handled
   in the kernel yet */
#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2
#define O_ACCMODE(x)	((x) & 3)
#define O_APPEND	4
#define O_SYNC		8
#define O_NDELAY	16
#define O_FLOCK		128		/* Cannot be user set */
#define O_CREAT		256
#define O_EXCL		512
#define O_TRUNC		1024
#define O_NOCTTY	2048
#define O_CLOEXEC	4096

#define O_BADBITS	(32 | 64 | O_FLOCK | 8192 | 16384 | 32768U)

#define F_GETFL		0
#define F_SETFL		1
#define F_GETFD		2
#define F_SETFD		3
#define F_DUPFD		4

#define FNDELAY		O_NDELAY

#define LOCK_SH		0
#define LOCK_EX		1
#define LOCK_UN		2		/* Must be highest */

#define LOCK_NB		O_NDELAY	/* Must be O_NDELAY */

/*
 * Error codes
 */
#define EPERM           1               /* Not owner */
#define ENOENT          2               /* No such file or directory */
#define ESRCH           3               /* No such process */
#define EINTR           4               /* Interrupted System Call */
#define EIO             5               /* I/O Error */
#define ENXIO           6               /* No such device or address */
#define E2BIG           7               /* Arg list too long */
#define ENOEXEC         8               /* Exec format error */
#define EBADF           9               /* Bad file number */
#define ECHILD          10              /* No children */
#define EAGAIN          11              /* No more processes */
#define ENOMEM          12              /* Not enough core */
#define EACCES          13              /* Permission denied */
#define EFAULT          14              /* Bad address */
#define ENOTBLK         15              /* Block device required */
#define EBUSY           16              /* Mount device busy */
#define EEXIST          17              /* File exists */
#define EXDEV           18              /* Cross-device link */
#define ENODEV          19              /* No such device */
#define ENOTDIR         20              /* Not a directory */
#define EISDIR          21              /* Is a directory */
#define EINVAL          22              /* Invalid argument */
#define ENFILE          23              /* File table overflow */
#define EMFILE          24              /* Too many open files */
#define ENOTTY          25              /* Not a typewriter */
#define ETXTBSY         26              /* Text file busy */
#define EFBIG           27              /* File too large */
#define ENOSPC          28              /* No space left on device */
#define ESPIPE          29              /* Illegal seek */
#define EROFS           30              /* Read-only file system */
#define EMLINK          31              /* Too many links */
#define EPIPE           32              /* Broken pipe */

/* math software */
#define EDOM            33              /* Argument too large */
#define ERANGE          34              /* Result too large */

#define EWOULDBLOCK	EAGAIN		/* Operation would block */
#define ENOLCK		35		/* Lock table full */
#define ENOTEMPTY	36		/* Directory is not empty */
#define ENAMETOOLONG    37              /* File name too long */

/*
 * ioctls for kernel internal operations start at 0x8000 and cannot be issued
 * by userspace.
 */
#define SELECT_BEGIN		0x8000
#define SELECT_END		0x8001

struct sysinfoblk {
  uint8_t infosize;		/* For expandability */
  uint8_t banks;		/* Banks in our 64K (and thus pagesize) */
  uint8_t max_open;
  uint8_t nproc;		/* Number of processes */
  uint16_t ticks;		/* Tick rate in HZ */
  uint16_t memk;		/* Memory in KB */
  uint16_t usedk;		/* Used memory in KB */
  uint16_t config;		/* Config flag mask */
#define CONF_PROFIL		1
#define CONF_NET		2	/* Hah.. 8) */
  uint16_t loadavg[3];
  uint32_t spare2;
    			        /* Followed by uname strings */
};

/* Select: if we do the map optimisation trick then we will want one bit free
   for optimising, so ideal PTABSIZE will become 1 below a multiple of 8 */

#define SELMAPSIZE  ((PTABSIZE+7)/8)

struct selmap {
  uint8_t map[SELMAPSIZE];
};

#define SELECT_IN		1
#define SELECT_OUT		2
#define SELECT_EX		4



/* functions in common memory */

/* debug functions */
COMMON void trap_monitor(void);
CODE1 void idump(void);

/* start.c */

/* platform/device.c */
CODE1 bool validdev(uint16_t dev);

/* usermem.c */
CODE2 usize_t valaddr(const char *base, usize_t size);
CODE2 int uget(const void *userspace_source, void *dest, usize_t count);
CODE2 int16_t  ugetc(const void *userspace_source);
CODE2 uint16_t ugetw(const void *userspace_source);
CODE2 int ugets(const void *userspace_source, void *dest, usize_t maxlen);
CODE2 int uput (const void *source,   void *userspace_dest, usize_t count);
CODE2 int uputc(uint16_t value,  void *userspace_dest);	/* u16_t so we don't get wacky 8bit stack games */
CODE2 int uputw(uint16_t value, void *userspace_dest);
CODE2 int uzero(void *userspace_dest, usize_t count);

/* usermem.c or usermem_std.s */
COMMON usize_t _uget(const uint8_t *user, uint8_t *dst, usize_t count);
COMMON int16_t _ugetc(const uint8_t *user);
COMMON uint16_t _ugetw(const uint16_t *user);
COMMON int _ugets(const uint8_t *user, uint8_t *dest, usize_t maxlen);
COMMON int _uput(const uint8_t *source, uint8_t *user, usize_t count);
COMMON int _uputc(uint16_t value,  uint8_t *user);
COMMON int _uputw(uint16_t value,  uint16_t *user);
COMMON int _uzero(uint8_t *user, usize_t count);

/* platform/tricks.s */
COMMON void switchout(void);
COMMON void doexec(uint16_t start_addr);
COMMON void switchin(ptptr process);
COMMON int16_t dofork(ptptr child);

/* devio.c */
CODE1 uint8_t *bread (uint16_t dev, blkno_t blk, bool rewrite);
CODE1 void brelse(void *bp);
CODE1 void bawrite(void *bp);
CODE1 int bfree(bufptr bp, uint8_t dirty); /* dirty: 0=clean, 1=dirty (write back), 2=dirty+immediate write */
CODE1 void *tmpbuf(void);
CODE1 void *zerobuf(void);
CODE1 void bufsync(void);
CODE1 bufptr bfind(uint16_t dev, blkno_t blk);
CODE1 bufptr freebuf(void);
CODE1 void bufinit(void);
CODE1 void bufdiscard(bufptr bp);
CODE1 void bufdump (void);
CODE1 int bdread(bufptr bp);
CODE1 int bdwrite(bufptr bp);
CODE1 int cdread(uint16_t dev, uint8_t flag);
CODE1 int d_open(uint16_t dev, uint8_t flag);
CODE1 int d_close(uint16_t dev);
CODE1 int d_ioctl(uint16_t dev, uint16_t request, char *data);
CODE1 int cdwrite(uint16_t dev, uint8_t flag);
CODE1 bool insq(struct s_queue *q, unsigned char c);
CODE1 bool remq(struct s_queue *q, unsigned char *cp);
CODE1 void clrq(struct s_queue *q);
CODE1 bool uninsq(struct s_queue *q, unsigned char *cp);
CODE1 int psleep_flags(void *event, unsigned char flags);
CODE1 int nxio_open(uint8_t minor, uint16_t flag);
CODE1 int no_open(uint8_t minor, uint16_t flag);
CODE1 int no_close(uint8_t minor);
CODE1 int no_rdwr(uint8_t minir, uint8_t rawflag, uint8_t flag);
CODE1 int no_ioctl(uint8_t minor, uint16_t a, char *b);

/* filesys.c */
/* open file, "name" in user address space */
CODE1 inoptr n_open(char *uname, inoptr *parent);
/* open file, "name" in kernel address space */
CODE1 inoptr kn_open(char *uname, inoptr *parent);
CODE1 inoptr i_open(uint16_t dev, uint16_t ino);
CODE1 inoptr srch_dir(inoptr wd, char *compname);
CODE1 inoptr srch_mt(inoptr ino);
CODE1 bool ch_link(inoptr wd, char *oldname, char *newname, inoptr nindex);
CODE1 void filename(char *userspace_upath, char *name);
/* return true if n1 == n2 */
CODE1 bool namecomp(char *n1, char *n2);
CODE1 inoptr newfile(inoptr pino, char *name);
CODE1 fsptr getdev(uint16_t dev);
CODE1 bool baddev(fsptr dev);
CODE1 uint16_t i_alloc(uint16_t devno);
CODE1 void i_free(uint16_t devno, uint16_t ino);
CODE1 blkno_t blk_alloc(uint16_t devno);
CODE1 void blk_free(uint16_t devno, blkno_t blk);
CODE1 int8_t oft_alloc(void);
CODE1 void deflock(struct oft *ofptr);
CODE1 void oft_deref(int8_t of);
/* returns index of slot, or -1 on failure */
CODE1 int8_t uf_alloc(void);
/* returns index of slot, or -1 on failure */
CODE1 int8_t uf_alloc_n(int n);
CODE1 void i_ref(inoptr ino);
CODE1 void i_deref(inoptr ino);
CODE1 void wr_inode(inoptr ino);
CODE1 bool isdevice(inoptr ino);
CODE1 void f_trunc(inoptr ino);
CODE1 void freeblk(uint16_t dev, blkno_t blk, uint8_t level);
CODE1 blkno_t bmap(inoptr ip, blkno_t bn, int rwflg);
CODE1 void validblk(uint16_t dev, blkno_t num);
CODE1 inoptr getinode(uint8_t uindex);
CODE1 bool super(void);
CODE1 bool esuper(void);
CODE1 uint8_t getperm(inoptr ino);
CODE1 void setftime(inoptr ino, uint8_t flag);
CODE1 uint16_t getmode(inoptr ino);
CODE1 struct mount *fs_tab_get(uint16_t dev);
/* returns true on failure, false on success */
CODE1 bool fmount(uint16_t dev, inoptr ino, uint16_t flags);
CODE1 void magic(inoptr ino);

/* inode.c */
CODE1 void readi(inoptr ino, uint8_t flag);
CODE1 void writei(inoptr ino, uint8_t flag);
CODE1 int16_t doclose (uint8_t uindex);
CODE1 inoptr rwsetup (bool is_read, uint8_t *flag);

/* mm.c */
CODE2 unsigned int uputsys(unsigned char *from, unsigned int size);
CODE2 unsigned int ugetsys(unsigned char *to, unsigned int size);

/* process.c */
CODE2 void psleep(void *event);
CODE2 void wakeup(void *event);
CODE2 void pwake(ptptr p);
CODE2 ptptr getproc(void);
CODE2 void newproc(ptptr p);
CODE2 ptptr ptab_alloc(void);
CODE2 void ssig(ptptr proc, uint16_t sig);
CODE2 void chksigs(void);
COMMON void program_vectors(uint16_t *pageptr);
CODE2 void sgrpsig(uint16_t pgrp, uint16_t sig);
CODE2 void unix_syscall(void);
CODE2 void timer_interrupt(void);
CODE2 void doexit (int16_t val, int16_t val2);
CODE2 void panic(char *deathcry);
CODE2 void exec_or_die(void);

/* select.c */
CODE2 extern void seladdwait(struct selmap *s);
CODE2 extern void selrmwait(struct selmap *s);
CODE2 extern void selwake(struct selmap *s);
#ifdef CONFIG_SELECT
CODE2 extern int selwait_inode(inoptr i, uint8_t smask, uint8_t setit);
CODE2 extern void selwake_inode(inoptr i, uint16_t mask);
CODE2 extern void selwake_pipe(inoptr i, uint16_t mask);
CODE2 extern int _select(void);
#else
#define selwait_inode(i,smask,setit) do {} while(0)
#define selwake_inode(i,smask,setit) do {} while(0)
#define selwake_pipe(i,smask,setit) do {} while(0)
#endif

/* swap.c */
extern ptptr swapproc;
extern uint8_t *swapbase;
extern unsigned int swapcnt;
extern blkno_t swapblk;

CODE2 extern void swapmap_add(uint8_t swap);
CODE2 extern ptptr swapneeded(ptptr p, int selfok);
CODE2 extern void swapper(ptptr p);

/* syscalls_fs.c, syscalls_proc.c, syscall_other.c etc */
CODE2 void updoff(void);
CODE2 int stcpy(inoptr ino, char *buf);
CODE2 bool rargs (char **userspace_argv, struct s_argblk *argbuf);
CODE2 char **wargs(char *userspace_ptr, struct s_argblk *argbuf, int  *cnt);
CODE2 extern int16_t unlinki(inoptr ino, inoptr pino, char *fname);

/* timer.c */
CODE2 void rdtime(time_t *tloc);
CODE2 void rdtime32(uint32_t *tloc);
CODE2 void wrtime(time_t *tloc);
CODE2 extern void updatetod(void);
CODE2 extern void inittod(void);

/* provided by architecture or helpers */
CODE2 void device_init(void);	/* provided by platform */
CODE2 void pagemap_init(void);
CODE2 void pagemap_add(uint8_t page);
CODE2 void pagemap_free(ptptr p);
CODE2 int pagemap_alloc(ptptr p);
CODE2 int pagemap_realloc(uint16_t p);
CODE2 uint16_t pagemap_mem_used(void);
CODE2 uint8_t *swapout_prepare_uarea(ptptr p);
CODE2 uint8_t *swapin_prepare_uarea(ptptr p);
CODE2 void map_init(void);
CODE2 void platform_idle(void);
CODE2 uint8_t rtc_secs(void);

/* Will need a uptr_t eventually */
extern uint16_t ramtop;	     /* Note: ramtop must be in common in some cases */
CODE2 extern void platform_interrupt(void);
COMMON void invalidate_cache(uint16_t page);
COMMON void flush_cache(ptptr p);

CODE2 int16_t __exit(void);        /* FUZIX system call 0 */
CODE2 int16_t _open(void);         /* FUZIX system call 1 */
CODE2 int16_t _close(void);        /* FUZIX system call 2 */
CODE2 int16_t _rename(void);       /* FUZIX system call 3 */
CODE2 int16_t _mknod(void);        /* FUZIX system call 4 */
CODE2 int16_t _link(void);         /* FUZIX system call 5 */
CODE2 int16_t _unlink(void);       /* FUZIX system call 6 */
CODE2 int16_t _read(void);         /* FUZIX system call 7 */
CODE2 int16_t _write(void);        /* FUZIX system call 8 */
CODE2 int16_t _lseek(void);        /* FUZIX system call 9 */
CODE2 int16_t _chdir(void);        /* FUZIX system call 10 */
CODE2 int16_t _sync(void);         /* FUZIX system call 11 */
CODE2 int16_t _access(void);       /* FUZIX system call 12 */
CODE2 int16_t _chmod(void);        /* FUZIX system call 13 */
CODE2 int16_t _chown(void);        /* FUZIX system call 14 */
CODE2 int16_t _stat(void);         /* FUZIX system call 15 */
CODE2 int16_t _fstat(void);        /* FUZIX system call 16 */
CODE2 int16_t _dup(void);          /* FUZIX system call 17 */
CODE2 int16_t _getpid(void);       /* FUZIX system call 18 */
CODE2 int16_t _getppid(void);      /* FUZIX system call 19 */
CODE2 int16_t _getuid(void);       /* FUZIX system call 20 */
CODE2 int16_t _umask(void);        /* FUZIX system call 21 */
CODE2 int16_t _getfsys(void);      /* FUZIX system call 22 */
CODE2 int16_t _execve(void);       /* FUZIX system call 23 */
CODE2 int16_t _getdirent(void);    /* FUZIX system call 24 */
CODE2 int16_t _setuid(void);       /* FUZIX system call 25 */
CODE2 int16_t _setgid(void);       /* FUZIX system call 26 */
CODE2 int16_t _time(void);         /* FUZIX system call 27 */
CODE2 int16_t _stime(void);        /* FUZIX system call 28 */
CODE2 int16_t _ioctl(void);        /* FUZIX system call 29 */
CODE2 int16_t _brk(void);          /* FUZIX system call 30 */
CODE2 int16_t _sbrk(void);         /* FUZIX system call 31 */
CODE2 int16_t _fork(void);         /* FUZIX system call 32 */
CODE2 int16_t _mount(void);        /* FUZIX system call 33 */
CODE2 int16_t _umount(void);       /* FUZIX system call 34 */
CODE2 int16_t _signal(void);       /* FUZIX system call 35 */
CODE2 int16_t _dup2(void);         /* FUZIX system call 36 */
CODE2 int16_t _pause(void);        /* FUZIX system call 37 */
CODE2 int16_t _alarm(void);        /* FUZIX system call 38 */
CODE2 int16_t _kill(void);         /* FUZIX system call 39 */
CODE2 int16_t _pipe(void);         /* FUZIX system call 40 */
CODE2 int16_t _getgid(void);       /* FUZIX system call 41 */
CODE2 int16_t _times(void);        /* FUZIX system call 42 */
CODE2 int16_t _utime(void);        /* FUZIX system call 43 */
CODE2 int16_t _geteuid(void);      /* FUZIX system call 44 */
CODE2 int16_t _getegid(void);      /* FUZIX system call 45 */
CODE2 int16_t _chroot(void);       /* FUZIX system call 46 */
CODE2 int16_t _fcntl(void);        /* FUZIX system call 47 */
CODE2 int16_t _fchdir(void);       /* FUZIX system call 48 */
CODE2 int16_t _fchmod(void);       /* FUZIX system call 49 */
CODE2 int16_t _fchown(void);       /* FUZIX system call 50 */
CODE2 int16_t _mkdir(void);	   /* FUZIX system call 51 */
CODE2 int16_t _rmdir(void);        /* FUZIX system call 52 */
CODE2 int16_t _setpgrp(void);	   /* FUZIX system call 53 */
CODE2 int16_t _uname(void);	   /* FUZIX system call 54 */
CODE2 int16_t _waitpid(void);	   /* FUZIX system call 55 */
CODE2 int16_t _profil(void);	   /* FUZIX system call 56 */
CODE2 int16_t _uadmin(void);	   /* FUZIX system call 57 */
CODE2 int16_t _nice(void);         /* FUZIX system call 58 */
CODE2 int16_t _sigdisp(void);	   /* FUZIX system call 59 */
CODE2 int16_t _flock(void);	   /* FUZIX system call 60 */

#endif /* __FUZIX__KERNEL_DOT_H__ */
