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

/* Import the CPU types before the config.h as config.h wants to use types */
#include "types.h"
#include "config.h"
#include "exec.h"

#include "cpu.h"

#include "panic.h"

#ifndef NULL
#define NULL (void *)0
#endif

#ifndef regptr
#define regptr
#endif

#define min(a,b) ( (a) < (b) ? (a) : (b) )
#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define aligndown(v,a) (uint8_t*)((intptr_t)(v) & ~((a)-1))
#define alignup(v,a) (uint8_t*)((intptr_t)((v) + (a)-1) & ~((a)-1))

/* By default, assume machines that don't need alignment. */

#ifndef ALIGNUP
#define ALIGNUP(v) (v)
#endif

#ifndef ALIGNDOWN
#define ALIGNDOWN(v) (v)
#endif

#ifndef FS_MAX_SHIFT
#define FS_MAX_SHIFT	0
#endif

/* These work fine for most compilers but can be overriden for those where the
   resulting code generation is foul */
#ifndef LOWORD
#define LOWORD(x)	((uint16_t)(x))
#endif
#ifndef HIBYTE32
#define HIBYTE32(x)	((uint8_t)((x) >> 24))
#endif

#ifndef CONFIG_BLOCK_SLEEP
#define i_unlock(x)	do {} while(0)
#define i_lock(x)	do {} while(0)
#define i_islocked(x)	do {} while(0)
#define i_unlock_deref(x)	i_deref(x)
#define n_open_lock(a,b)	n_open((a),(b))
#define getinode_lock(x)	getinode(x)
#endif

#ifdef CONFIG_LEVEL_2
#include "level2.h"
#else

#define jobcontrol_in(x,y)	0
#define jobcontrol_out(x,y)	0
#define jobcontrol_ioctl(x,y,z)	0

#define limit_exceeded(x,y) (0)
#define can_signal(p, sig) \
	(udata.u_ptab->p_uid == (p)->p_uid || super())
#define dump_core(sig)	sig
#define in_group(x)	0
#endif

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
#ifndef MAPBASE		/* Usually the start of program and map match */
#define MAPBASE PROGBASE
#endif

#ifndef NGROUP
#define NGROUP		16
#endif


/* Default to longer slices. For most ports it's a better choice */
#ifndef MAXTICKS
#define MAXTICKS     (TICKSPERSEC/2)
                           /* Max ticks before switching out (time slice)
                              default process time slice */
#endif

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

#if !defined(CONFIG_INDIRECT_QUEUES)
typedef unsigned char * queueptr_t;
#endif

typedef struct s_queue {
    queueptr_t q_base;    /* Pointer to data */
    queueptr_t q_head;    /* Pointer to addr of next char to read. */
    queueptr_t q_tail;    /* Pointer to where next char to insert goes. */
    int   q_size;    /* Max size of queue */
    int   q_count;   /* How many characters presently in queue */
    int   q_wakeup;  /* Threshold for waking up processes waiting on queue */
} queue_t;

#if !defined CONFIG_INDIRECT_QUEUES
	#define GETQ(p) (*(p))
	#define PUTQ(p, v) (*(p) = (v))
#endif

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
typedef uint32_t uoff_t;	/* Internal use so we can keep the compiler happy */

typedef uint16_t blkno_t;    /* Can have 65536 512-byte blocks in filesystem */
#define NULLBLK ((blkno_t)-1)

#define BLKSIZE		512
#define BLKSHIFT	9
#define BLKMASK		511
#define BLKOVERSIZE32	0xFE	/* Bits 25+ mean we exceeded the file size */

/* Help the 8bit compilers out by preventing any 32bit promotions */
#define BLKOFF(x)	(((uint16_t)(x)) & BLKMASK)

/* State of the block. We have some free bits here if we need them */
#define BF_FREE		0
#define BF_BUSY		1

#if !defined(CONFIG_BLKBUF_EXTERNAL)
#define CONFIG_BLKBUF_HELPERS
#endif

typedef struct blkbuf {
#ifdef CONFIG_BLKBUF_EXTERNAL
    uint8_t	*__bf_data;
#else
    uint8_t     __bf_data[BLKSIZE];    /* This MUST be first ! */
#endif
    uint16_t    bf_dev;
    blkno_t     bf_blk;
    uint8_t     bf_dirty;	/* bit 0 used */
    uint8_t     bf_busy;	/* bits 0-1 used */
    uint16_t    bf_time;        /* LRU time stamp */
} blkbuf, *bufptr;

#if defined(CONFIG_BLKBUF_HELPERS)
#define blktok(kaddr,buf,off,len) \
    memcpy((kaddr), (buf)->__bf_data + (off), (len))
#define blkfromk(kaddr,buf, off,len) \
    memcpy((buf)->__bf_data + (off), (kaddr), (len))
#define blktou(uaddr,buf,off,len) \
    uput((buf)->__bf_data + (off), (uaddr), (len))
#define blkfromu(uaddr,buf,off,len) \
    uget((uaddr),(buf)->__bf_data + (off), (len))
#define blkptr(buf, off, len)	((void *)((buf)->__bf_data + (off)))
#define blkzero(buf)		memset(buf->__bf_data, 0, BLKSIZE)
#else
extern void blktok(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len);
extern void blkfromk(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len);
extern void blktou(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len);
extern void blkfromu(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len);
/* Worst case is needing to copy over about 64 bytes */
extern void *blkptr(struct blkbuf *buf, uint16_t offset, uint16_t len);
extern void blkzero(struct blkbuf *buf);
#endif

/* TODO: consider smaller inodes or clever caching. 2BSD uses small
   direct block lists to keep inodes small as they must be in memory when
   'live' - we could also split them into the 28 bytes we always need to
   keep live (i_addr[0] for the dev ptr) and the 36 we don't (or 32/32 for
   speed). We'd then be able to drop half the bits for an open inode onto
   disk safely */
typedef struct dinode {
    uint16_t i_mode;
    uint16_t i_nlink;		/* Note we have 64K inodes so we never overflow */
    uint16_t i_uid;
    uint16_t i_gid;
    uoff_t    i_size;		/* Never negative */
    uint32_t   i_atime;		/* Breaks in 2038 */
    uint32_t   i_mtime;		/* Need to hide some extra bits ? */
    uint32_t   i_ctime;		/* 24 bytes */
    blkno_t  i_addr[20];
} dinode;               /* Exactly 64 bytes long! */

/* We use the Linux one for compatibility. There's no real Unix 'standard'
   for such things */

struct hd_geometry {
	uint8_t heads;
	uint8_t sectors;
	uint16_t cylinders;
	uint32_t start;
};

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

/* So we can do all our getmode comparisons in 8bit to help the compilers out */
#define MODE_R(x)	((uint8_t)((x) >> 8))

#define major(x) ((x) >> 8)
#define minor(x) ((x) & 0xFF)

/* In memory inode structure */
typedef struct cinode {
    uint16_t   c_magic;         /* Used to check for corruption. */
    uint16_t   c_dev;           /* Inode's device */
    uint16_t   c_num;           /* Inode # */
    dinode     c_node;		/* On disk inode data */
    uint8_t    c_refs;          /* In-core reference count */
    uint8_t    c_readers;	/* Count of readers by oft entry */
    uint8_t    c_writers;	/* Count of writers by oft entry */
    uint8_t    c_flags;           
#define CDIRTY		0x80	/* Modified flag. */
#define CRDONLY		0x40	/* On a read only file system */
#define CFLOCK		0x0F	/* flock bits */
#define CFLEX		0x0F	/* locked exclusive */
#define CFMAX		0x0E	/* highest shared lock count permitted */
   uint8_t     c_super;		/* Superblock index */
#ifdef CONFIG_BLOCK_SLEEP
   uint16_t    c_lock;		/* inode lock state */
#endif
} cinode, *inoptr;

#define NULLINODE ((inoptr)NULL)
#define NULLINOPTR ((inoptr*)NULL)

#define FILENAME_LEN	30
#define DIR_LEN		32
typedef struct direct {
    uint16_t   d_ino;
    uint8_t    d_name[FILENAME_LEN];
} direct;


/*
 * Superblock structure
 */
#define FILESYS_TABSIZE 50
typedef struct filesys { // note: exists in mem and on disk
    uint16_t      s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    uint16_t      s_nfree;
    blkno_t       s_free[FILESYS_TABSIZE];
    int16_t       s_ninode;
    uint16_t      s_inode[FILESYS_TABSIZE];
    uint8_t       s_fmod;
    /* 0 is 'legacy' and never written to disk */
#define FMOD_GO_CLEAN	0	/* Write a clean to the disk (internal) */
#define FMOD_DIRTY	1	/* Mounted or uncleanly unmounted from r/w */
#define FMOD_CLEAN	2	/* Clean. Used internally to mean don't
				   update the super block */
    uint8_t       s_timeh;	/* bits 32-40: FIXME - wire up */
    uint32_t      s_time;
    blkno_t       s_tfree;
    uint16_t      s_tinode;
    uint8_t	  s_shift;	/* Extent size */
} filesys, *fsptr;

/*
 * Superblock with userspace fields that are not kept in the kernel
 * mount table.
 */
struct filesys_user {
    struct filesys s_fs;
    /* Allow for some kernel expansion */
    uint8_t	  s_reserved;
    uint16_t	  s_reserved2[16];
    /* This is only used by userspace */
    uint16_t	  s_props;	/* Property bits indicating which are valid */
#define S_PROP_LABEL	1
#define S_PROP_GEO	2
    /* For now only one property set - geometry. We'll eventually use this
       when we don't know physical geometry and need to handle stuff with
       tools etc */
    uint8_t	  s_label_name[32];

    uint16_t      s_geo_heads;	/* If 0/0/0 is specified and valid it means */
    uint16_t	  s_geo_cylinders; /* pure LBA - no idea of geometry */
    uint16_t	  s_geo_sectors;
    uint8_t	  s_geo_skew;	/* Soft skew if present (for hard sectored media) */
                                /* Gives the skew (1/2/3/4/5/... etc) */
    uint8_t	  s_geo_secsize;/* Physical sector size in log2 form*/
};

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
    inoptr   m_mntpt;     /* Mount point */
    struct filesys m_fs;
};
#define MS_RDONLY	1
#define MS_NOSUID	2	/* Not yet implemented */
#define MS_NOEXEC	4	/* Not yet implemented */
#define MS_REMOUNT	128

/* Process table p_status values */

#define P_EMPTY         0    /* Unused slot */
#define P_RUNNING       1    /* Currently running process (must match value in kernel.def) */
/* The sleeping range must be together see swap.c */
#define P_READY         2    /* Runnable   */
#define P_SLEEP         3    /* Sleeping; can be awakened by signal */
#define P_IOWAIT	4    /* Sleeping: don't wake for a signal */
#define P_STOPPED       5    /* Sleeping, signal driven halt */
#define P_FORKING       6    /* In process of forking; do not mess with */
#define P_ZOMBIE        7    /* Exited. */
#define P_NOSLEEP	8    /* In an internal state where sleep is forbidden */


/* 0 is used to mean 'check we could signal this process' */

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

#define  SIG_DFL   (int (*)(int))0	/* Must be 0 */
#define  SIG_IGN   (int (*)(int))1

#define sigmask(sig)    (1U<<((sig) & 0x0F))

/* uadmin */

#define A_SHUTDOWN		1
#define A_REBOOT		2
#define A_DUMP			3
#define A_FREEZE		4	/* Unimplemented, want for NC100? */
#define A_SWAPCTL		16	/* Unimplemented */
#define A_CONFIG		17	/* Unimplemented */
#define A_FTRACE		18	/* Unimplemented: 
                                          Hook to the syscall trace debug */
#define A_SUSPEND               32	/* Suspend to RAM (optional) */

#define AD_NOSYNC		1

#define A_SC_ADD		1

                                          
/* Process table entry */

struct sigbits {
    uint16_t	s_pending;
    uint16_t	s_ignored;
    uint16_t	s_held;
};

typedef struct p_tab {
    /* WRS: UPDATE kernel.def IF YOU CHANGE THIS STRUCTURE */
    uint8_t     p_status;       /* Process status: MUST BE FIRST MEMBER OF STRUCT */
    uint8_t	p_flags;	/* Bitflags: must be adjacent */
#define PFL_CHKSIG	1	/* Signal check required */
#define PFL_ALARM	2	/* On alarm queue */
#define PFL_BATCH	4	/* Used full time quantum */
    uint8_t     p_tty;          /* Process' controlling tty minor # */
    uint16_t    p_pid;          /* Process ID */
    uint16_t    p_uid;
    struct p_tab *p_pptr;      /* Process parent's table entry */
    uarg_t      p_alarm;        /* Deciseconds until alarm goes off */
    uint16_t    p_exitval;      /* Exit value */
    void *      p_wait;         /* Address of thing waited for */
    uint16_t    p_page;         /* Page mapping data */
    uint16_t	p_page2;	/* It's really four bytes for the platform */
#ifdef udata
    struct u_data *p_udata;	/* Udata pointer for platforms using dynamic udata */
#endif
    /* Update kernel.def if you change fields above this comment */
    uint16_t    p_priority;     /* Process priority */
    struct sigbits p_sig[2];
    uint16_t    p_waitno;       /* wait #; for finding longest waiting proc */
    uint16_t    p_timeout;      /* timeout in centiseconds - 1 */
                                /* 0 indicates no timeout, 1 = expired */

/**HP**/
    char    p_name[8];
    clock_t p_time, p_utime, p_stime, p_cutime, p_cstime;
/**HP**/
    uint16_t	p_pgrp;		/* Process group */
    uint8_t	p_nice;
    uint8_t	p_event;	/* Events */
    usize_t	p_top;		/* Top of user memory */
#ifdef CONFIG_LEVEL_2
    uint16_t	p_session;
#endif
#ifdef CONFIG_PROFIL
    uint8_t	p_profscale;
    void *	p_profbuf;
    uaddr_t	p_profsize;
    uaddr_t	p_profoff;
#endif    
    /* Put new stuff we don't care about in asm or ps at the end */
    struct p_tab *p_timerq;
} p_tab, *ptptr;

/*
 *	We copy the u_data block between processes when we fork and on some
 *	platforms it moves. This means that there must be *no* internal pointer
 *	references to the udata within u_data itself.
 */
typedef struct u_data {
    /**** If you change this top section, also update offsets in "kernel.def" ****/
    struct p_tab *u_ptab;       /* Process table pointer */
    uint16_t    u_page;         /* Process page data (equal to u_ptab->p_page) */
    uint16_t    u_page2;        /* Process page data (equal to u_ptab->p_page2) */
    bool        u_insys;        /* True if in kernel */
    uint8_t     u_callno;       /* sys call being executed. */
    uaddr_t     u_syscall_sp;   /* Stores SP when process makes system call */
    susize_t    u_retval;       /* Return value from sys call */
    int16_t     u_error;        /* Last error number */
    void *      u_sp;           /* Stores SP when process is switchped */
    bool        u_ininterrupt;  /* True when the interrupt handler is running (prevents recursive interrupts) */
    int8_t      u_cursig;       /* Next signal to be dispatched */
    arg_t       u_argn;         /* First C argument to the system call */
    arg_t       u_argn1;        /* Second C argument */
    arg_t       u_argn2;	/* Third C argument */
    arg_t       u_argn3;        /* Fourth C argument */
    void *      u_isp;          /* Value of initial sp (argv) */
    uaddr_t	u_break;	/* Top of data space */
#ifdef CONFIG_32BIT
    uaddr_t	u_codebase;	/* 32bit platform base pointers */
#endif
    int     (*u_sigvec[NSIGS])(int);   /* Array of signal vectors */

    uint8_t *   u_base;         /* Source or dest for I/O */
    usize_t     u_count;        /* Amount for I/O */
    off_t       u_offset;       /* Place in file for I/O */
    /**** If you change this top section, also update offsets in "kernel.def" ****/

    struct blkbuf *u_buf;
    bool        u_sysio;        /* True if I/O to system space */

    /* This block gets written to acct */
    
    /* We overwrite u_mask with p->p_uid on the exit */
    uint16_t    u_mask;         /* umask: file creation mode mask */
    uint16_t    u_gid;
    uint16_t    u_euid;
    uint16_t    u_egid;
    char        u_name[8];      /* Name invoked with */
    
    /* This section is not written out except as padding */
    uint8_t     u_files[UFTSIZE];       /* Process file table: indices into open file table, or NO_FILE. */
    uint16_t	u_cloexec;	/* Close on exec flags */
    inoptr      u_cwd;          /* Index into inode table of cwd. */
    inoptr	u_root;		/* Index into inode table of / */
    inoptr	u_rename;	/* Used in n_open for rename() checking */
    inoptr	u_ctty;		/* Controlling tty */

    /* Temporaries used for block I/O */
    blkno_t	u_block;	/* Block number */
    uint16_t	u_blkoff;	/* Offset in block */
    usize_t	u_nblock;	/* Number of blocks */
    uint8_t	*u_dptr;	/* Address for I/O */
    usize_t	u_done;		/* Counter for driver methods */

#ifdef CONFIG_UDATA_TEXTTOP
	uaddr_t u_texttop;  /* Top of binary text (used for I/D systems) */
#endif
#ifdef CONFIG_LEVEL_2
    uint16_t    u_groups[NGROUP]; /* Group list */
    uint8_t	u_ngroup;
    uint8_t	u_flags;
#define U_FLAG_NOCORE		1	/* Set if no core dump */
    struct rlimit u_rlimit[NRLIMIT];	/* Resource limits */
#endif
} u_data;


/* This is the user data structure, padded out to 512 bytes with the
 * System Stack.
 */

#ifndef CONFIG_STACKSIZE
#define CONFIG_STACKSIZE 512
#endif

typedef struct u_block {
        u_data u_d;
        char   u_s [CONFIG_STACKSIZE - sizeof(struct u_data)];
} u_block;


/* Struct to temporarily hold arguments in execve */
struct s_argblk {
    int a_argc;
    int a_arglen;
    int a_envc;
    uint8_t a_buf[512-3*sizeof(int)];
};


/* waitpid options */
#define WNOHANG		1
#define WUNTRACED	2
#define _WSTOPPED	0xFF
#define W_COREDUMP	0x80

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
#define O_DIRECT	32
#define O_FLOCK		128		/* Cannot be user set */
#define O_CREAT		256
#define O_EXCL		512
#define O_TRUNC		1024
#define O_NOCTTY	2048
#define O_CLOEXEC	4096

#define O_BADBITS	(64 | O_FLOCK | 8192 | 16384 | 32768U)

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
#define EAFNOSUPPORT	38		/* Address family not supported */
#define EALREADY	39		/* Operation already in progress */
#define EADDRINUSE	40		/* Address already in use */
#define EADDRNOTAVAIL	41		/* Address not available */
#define ENOSYS		42		/* No such system call */
#define EPFNOSUPPORT	43		/* Protocol not supported */
#define EOPNOTSUPP	44		/* Operation not supported on transport
                                           endpoint */
#define ECONNRESET	45		/* Connection reset by peer */
#define ENETDOWN	46		/* Network is down */
#define EMSGSIZE	47		/* Message too long */

#define ETIMEDOUT	48		/* Connection timed out */
#define ECONNREFUSED	49		/* Connection refused */
#define EHOSTUNREACH	50		/* No route to host */
#define EHOSTDOWN	51		/* Host is down */
#define	ENETUNREACH	52		/* Network is unreachable */
#define ENOTCONN	53		/* Transport endpoint is not connected */
#define EINPROGRESS	54		/* Operation now in progress */
#define ESHUTDOWN	55		/* Cannot send after transport endpoint shutdown */
#define EISCONN         56              /* Socket is already connected */
#define EDESTADDRREQ    57              /* No destination address specified */

/*
 * ioctls for kernel internal operations start at 0x8000 and cannot be issued
 * by userspace.
 */
#define SELECT_BEGIN		0x8000
#define SELECT_TEST		0x8001
#define SELECT_END		0x8002

#define IOCTL_NORMAL		0x0000	/* No special rules */
#define IOCTL_SUPER		0x4000	/* Superuser needed */
#define IOCTL_KERNEL		0x8000	/* Kernel side only */
#define IOCTL_WRONLY		0xC000	/* Write handle needed */

#define IOCTL_CLASS_SUPER	0x40
#define IOCTL_CLASS_KERNEL	0x80
#define IOCTL_CLASS_WRONLY	0xC0

/*
 *	Ioctl ranges
 *	00xx		-	tty/vt
 *	01xx		-	disk
 */

/*
 *	TTY ioctls 00xx (see tty.h)
 */

/*
 *	Disk ioctls 01xx
 */

#define HDIO_GETGEO		0x0101
#define HDIO_GET_IDENTITY	0x0102	/* Not yet implemented anywhere */
#define BLKFLSBUF		0x4103	/* Use the Linux name */
#define HDIO_RAWCMD		0x4104	/* Issue a raw command, ioctl data
                                           is device dependent */
#define HDIO_EJECT		0x0105	/* Request a media eject */
#define HDIO_TRIM       0x0106  /* Issue a TRIM request */
#define BLKGETSIZE      0x0107  /* Use the Linux name */

/*
 *	Floppy disk ioctl s0x01Fx (see fdc.h)
 */

/*
 *	Sound ioctls 02xx (see audio.h)
 */


/*
 *	Graphics ioctls 03xx (see graphics.h)
 */

/*
 *	Networking ioctls 04xx (see net_native.h)
 */

/*
 *	Drivewire ioctls 050x (see drivewire.h)
 */

/*
 *	Input ioctls 0x052x (see input.h)
 */

/*
 *	GPIO ioctls 0x053x (see gpio.h)
 */

/*
 *	RTC ioctls 0x053x (see rtc.h)
 */

/*
 *	Tape ioctls 0x06xx (see tape.h)
 */

/*
 *	CPU and platform ioctls (see platform code)
 *	0700-07		8080-eZ80
 *			0700	Get kernel CPM hook info
 *	0708-0F		6502/65C816
 *	0710-17		6309/809
 *			0710	Hook SWI2 etc for emulators
 *	0718-1F		680x0
 *
 *	07Ex		Platform specific (check 07F0 before using as will
 *			be duplicated per platform)
 *	07Fx		Generic
 *			07F0	Get platform name string (unique)
 *				required if have non CPU specific ioctls
 *			07F1	Cache writeback/flush
 */

/*
 *	Printer ioctls 0x08xx (see printer.h)
 */


/*
 *	System info shared with user space
 */
#include <sysinfoblk.h>

/* Select: if we do the map optimisation trick then we will want one bit free
   for optimising, so ideal PTABSIZE will become 1 below a multiple of 8 */

#define SELMAPSIZE  ((PTABSIZE+7)/8)

struct selmap {
  uint8_t map[SELMAPSIZE];
};

#define SELECT_IN		1
#define SELECT_OUT		2
#define SELECT_EX		4

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#endif

/* functions in common memory */

/* debug functions */
extern void idump(void);

/* platform/device.c */
extern bool validdev(uint16_t dev);

/* usermem.c */
#ifdef CONFIG_LEVEL_0
extern size_t strlcpy(char *, const char *, size_t);
#define valaddr(a,b)	(1)
#define uget(a,b,c)	(memcpy(b,a,c) && 0)
#define uput(a,b,c)	(memcpy(b,a,c) && 0)
#define ugetc(a)	(*(uint8_t *)(a))
#define _ugetc(a)	(*(uint8_t *)(a))
#define ugetw(a)	(*(uint8_t *)(a))
#define uputc(v, p)	((*(uint8_t*)(p) = (v)) && 0)
#define uputw(v, p)	((*(uint16_t*)(p) = (v)) && 0)
#define uzero(a,b)	(memset(a,0,b) && 0)
#else
extern usize_t valaddr(const uint8_t *base, usize_t size);
extern int uget(const void *userspace_source, void *dest, usize_t count);
extern int16_t  ugetc(const void *userspace_source);
extern uint16_t ugetw(const void *userspace_source);
extern uint32_t _ugetl(void *uaddr);
extern int uput (const void *source,   void *userspace_dest, usize_t count);
extern int uputc(uint16_t value,  void *userspace_dest);	/* u16_t so we don't get wacky 8bit stack games */
extern int uputw(uint16_t value, void *userspace_dest);
extern int _uputl(uint32_t val, void *uaddr);
extern int uzero(void *userspace_dest, usize_t count);

/* usermem.c or usermem_std.s */
extern usize_t _uget(const uint8_t *user, uint8_t *dst, usize_t count);
extern int _uput(const uint8_t *source, uint8_t *user, usize_t count);
extern int _uzero(uint8_t *user, usize_t count);

#if defined CONFIG_USERMEM_DIRECT
#define _ugetc(p) (*(uint8_t*)(p))
#define _ugetw(p) (*(uint16_t*)(p))
#define _uputc(v, p) ((*(uint8_t*)(p) = (v)), 0)
#define _uputw(v, p) ((*(uint16_t*)(p) = (v)), 0)
#else
extern int16_t _ugetc(const uint8_t *user) __fastcall;
extern uint16_t _ugetw(const uint16_t *user) __fastcall;
extern int _uputc(uint16_t value,  uint8_t *user);
extern int _uputw(uint16_t value,  uint16_t *user);
#endif
#endif

/* platform/tricks.s */
extern void switchout(void);
extern void doexec(uaddr_t start_addr);
extern void switchin(ptptr process);
extern int16_t dofork(ptptr child);
extern uint8_t need_resched;

/* devio.c */
extern void validchk(uint16_t dev, const char *p);
extern bufptr bread (uint16_t dev, blkno_t blk, bool rewrite);
extern void brelse(bufptr);
extern void bawrite(bufptr);
extern int bfree(bufptr bp, uint_fast8_t dirty); /* dirty: 0=clean, 1=dirty (write back), 2=dirty+immediate write */
extern void *tmpbuf(void);
extern void tmpfree(void *p);
extern bufptr zerobuf(void);
extern void bufsync(void);
extern bufptr bfind(uint16_t dev, blkno_t blk);
extern void bdrop(uint16_t dev);
extern bufptr freebuf(void);
extern void bufinit(void);
extern void bufdump (void);
extern int bdread(bufptr bp);
extern int bdwrite(bufptr bp);
extern int cdread(uint16_t dev, uint_fast8_t flag);
extern int d_open(uint16_t dev, uint_fast8_t flag);
extern int d_close(uint16_t dev);
extern int d_ioctl(uint16_t dev, uint16_t request, char *data);
extern int d_flush(uint16_t dev);
extern int d_blkoff(uint_fast8_t bits);
extern int cdwrite(uint16_t dev, uint_fast8_t flag);
extern bool insq(struct s_queue *q, uint_fast8_t c);
extern bool remq(struct s_queue *q, uint_fast8_t *cp);
extern void clrq(struct s_queue *q);
extern bool uninsq(struct s_queue *q, uint_fast8_t *cp);
extern bool fullq(struct s_queue *q);
extern int psleep_flags_io(void *event, uint_fast8_t flags);
extern int psleep_flags(void *event, uint_fast8_t flags);
extern int nxio_open(uint_fast8_t minor, uint16_t flag);
extern int no_open(uint_fast8_t minor, uint16_t flag);
extern int no_close(uint_fast8_t minor);
extern int no_rdwr(uint_fast8_t minir, uint_fast8_t rawflag, uint_fast8_t flag);
extern int no_ioctl(uint_fast8_t minor, uarg_t a, char *b);

/* filesys.c */
/* open file, "name" in user address space */
extern uint8_t lastname[31];
extern inoptr n_open(uint8_t *uname, inoptr *parent);
extern inoptr i_open(uint16_t dev, uint16_t ino);
extern inoptr srch_dir(inoptr wd, uint8_t *compname);
extern inoptr srch_mt(inoptr ino);
extern bool emptydir(inoptr ino);
extern bool ch_link(inoptr wd, uint8_t *oldname, uint8_t *newname, inoptr nindex);
/* return true if n1 == n2 */
extern bool namecomp(uint8_t *n1, uint8_t *n2);
extern inoptr newfile(inoptr pino, uint8_t *name);
extern fsptr getdev(uint16_t dev);
extern bool baddev(fsptr dev);
extern uint16_t i_alloc(uint16_t devno);
extern void i_free(uint16_t devno, uint16_t ino);
extern blkno_t blk_alloc(uint16_t devno);
extern void blk_free(uint16_t devno, blkno_t blk);
extern int_fast8_t oft_alloc(void);
extern void deflock(struct oft *ofptr);
extern void oft_deref(uint_fast8_t of);
/* returns index of slot, or -1 on failure */
extern int_fast8_t uf_alloc(void);
/* returns index of slot, or -1 on failure */
extern int_fast8_t uf_alloc_n(uint_fast8_t n);
#define i_ref(ino) ((ino)->c_refs++, (ino))
//extern void i_ref(inoptr ino);
extern void i_deref(inoptr ino);
extern void wr_inode(inoptr ino);
extern bool isdevice(inoptr ino);
extern int f_trunc(inoptr ino);
extern void freeblk(uint16_t dev, blkno_t blk, uint_fast8_t level);
extern blkno_t bmap(inoptr ip, blkno_t bn, unsigned int rwflg);
extern void validblk(uint16_t dev, blkno_t num);
extern inoptr getinode(uint_fast8_t uindex);
extern bool super(void);
extern bool esuper(void);
extern uint8_t getperm(inoptr ino);
extern void setftime(inoptr ino, uint_fast8_t flag);
extern uint8_t getmode(inoptr ino);
extern struct mount *fs_tab_get(uint16_t dev);
extern struct mount *fmount(uint16_t dev, inoptr ino, uint16_t flags);
extern void magic(inoptr ino);
extern arg_t unlinki(inoptr ino, inoptr pino, uint8_t *fname);

/* inode.c */
extern void readi(inoptr ino, uint_fast8_t flag);
extern uint16_t umove(uint16_t n);	/* Probably wants to move ? */
extern void writei(inoptr ino, uint_fast8_t flag);
extern int16_t doclose (uint_fast8_t uindex);
extern inoptr rwsetup (bool is_read, uint_fast8_t *flag);
extern int dev_openi(inoptr *ino, uint16_t flag);
extern void sync(void);

/* mm.c */
extern unsigned int uputblk(bufptr bp, usize_t to, usize_t size);
extern unsigned int ugetblk(bufptr bp, usize_t from, usize_t size);

/* process.c */
extern void psleep(void *event);
extern void psleep_nosig(void *event);
extern void wakeup(void *event);
extern void pwake(ptptr p);
extern ptptr getproc(void);
extern void makeproc(ptptr p, u_data *u);
extern ptptr ptab_alloc(void);
extern void ssig(ptptr proc, uint_fast8_t sig);
extern void recalc_cursig(void);
extern uint_fast8_t chksigs(void);
extern void program_vectors(uint16_t *pageptr);
extern void sgrpsig(uint16_t pgrp, uint_fast8_t sig);
extern void unix_syscall(void);
extern void ptimer_insert(void);
extern void timer_interrupt(void);
extern void doexit (uint16_t val);
extern void NORETURN panic(char *deathcry);
extern void exec_or_die(void);
#define need_reschedule() (nready != 1 && runticks >= udata.u_ptab->p_priority)

#ifdef CONFIG_LEVEL_2
extern uint_fast8_t dump_core(uint8_t sig);
#endif

/* select.c */
#ifdef CONFIG_LEVEL_2
extern void seladdwait(struct selmap *s);
extern void selrmwait(struct selmap *s);
extern void selwake(struct selmap *s);
extern void selwait_inode(inoptr i, uint_fast8_t smask, uint_fast8_t setit);
extern void selwake_inode(inoptr i, uint16_t mask);
extern void selwake_pipe(inoptr i, uint16_t mask);
extern void selwake_dev(uint_fast8_t major, uint_fast8_t minor, uint16_t mask);
extern arg_t _select(void);
#else
#define selwait_inode(i,smask,setit) do {} while(0)
#define selwake_inode(i,smask) do {} while(0)
#define selwake_pipe(i,smask) do {} while(0)
#define selwake_dev(major,minor,smask) do {} while(0)
#endif

/* start.c */
extern void fuzix_main(void);
extern void set_boot_line(const char *p);

/* swap.c */
extern uint16_t swappage;

extern int swapread(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    uaddr_t buf, uint16_t page);
extern int swapwrite(uint16_t dev, blkno_t blkno, usize_t nbytes,
		     uaddr_t buf, uint16_t page);

extern void swapmap_add(uint_fast8_t swap);
extern void swapmap_init(uint_fast8_t swap);
extern int swapmap_alloc(void);
extern ptptr swapneeded(ptptr p, int selfok);
extern void swapper(ptptr p);
extern void swapper2(ptptr p, uint16_t map);
extern uint8_t get_common(void);
extern void swap_finish(uint_fast8_t page, ptptr p);
/* These two are provided by the bank code selected for the port */
extern int swapout(ptptr p);
extern void swapin(ptptr p, uint16_t map);

/* syscalls_fs.c, syscalls_proc.c, syscall_other.c etc */
extern void updoff(void);
extern int stcpy(inoptr ino, uint8_t *buf);
extern bool rargs (char **userspace_argv, struct s_argblk *argbuf);
extern char **wargs(char *userspace_ptr, struct s_argblk *argbuf, int  *cnt);

/* timer.c */
extern void rdtime(time_t *tloc);
extern void rdtime32(uint32_t *tloc);
extern void wrtime(time_t *tloc);
extern void updatetod(void);
extern void inittod(void);
extern void sync_clock(void);

/* provided by architecture or helpers */
extern void device_init(void);	/* provided by platform */
extern void pagemap_init(void);
extern void copy_common(uint8_t page);
extern void pagemap_add(uint8_t page);	/* FIXME: may need a page type for big boxes */
extern void pagemap_free(ptptr p);
extern int pagemap_alloc(ptptr p);
extern int pagemap_prepare(struct exec *hdr);
extern int pagemap_realloc(struct exec *hdr, usize_t m);
extern usize_t pagemap_mem_used(void);
extern void map_init(void);
extern void set_cpu_type(void);

/* Executable header checks and stubs */
extern uint8_t sys_cpu, sys_cpu_feat;
extern uint8_t sys_stubs[];

/* Platform interfaces */

#ifndef platform_discard
extern void platform_discard(void);
#endif
#ifndef platform_copyright
extern void platform_copyright(void);
#endif
extern void platform_idle(void);
extern uint_fast8_t platform_rtc_secs(void);
extern int platform_rtc_read(void);
extern int platform_rtc_write(void);
extern int platform_rtc_ioctl(uarg_t request, char *data);
extern void platform_reboot(void);
extern void platform_monitor(void);
extern uint_fast8_t platform_param(char *p);
extern void platform_switchout(void);
extern void platform_interrupt(void);
extern uint_fast8_t platform_suspend(void);
extern uint_fast8_t platform_udata_set(ptptr p);

extern void platform_swap_found(uint_fast8_t part, uint_fast8_t letter);
extern uint_fast8_t platform_canswapon(uint16_t devno);

extern int platform_dev_ioctl(uarg_t request, char *data);

extern uint8_t platform_tick_present;

#ifndef CONFIG_INLINE_IRQ
extern irqflags_t __hard_di(void);
extern void __hard_irqrestore(irqflags_t f);
extern void __hard_ei(void);
#endif

#ifndef CONFIG_SOFT_IRQ
#define di __hard_di
#define irqrestore __hard_irqrestore
#define ei __hard_ei
#else
extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);
extern void ei(void);
#endif

/* Will need a uptr_t eventually */
extern uaddr_t ramtop;	     /* Note: ramtop must be in common in some cases */
extern void invalidate_cache(uint16_t page);
extern void flush_cache(ptptr p);

extern arg_t __exit(void);        /* FUZIX system call 0 */
extern arg_t _open(void);         /* FUZIX system call 1 */
extern arg_t _close(void);        /* FUZIX system call 2 */
extern arg_t _rename(void);       /* FUZIX system call 3 */
extern arg_t _mknod(void);        /* FUZIX system call 4 */
extern arg_t _link(void);         /* FUZIX system call 5 */
extern arg_t _unlink(void);       /* FUZIX system call 6 */
extern arg_t _read(void);         /* FUZIX system call 7 */
extern arg_t _write(void);        /* FUZIX system call 8 */
extern arg_t _lseek(void);        /* FUZIX system call 9 */
extern arg_t _chdir(void);        /* FUZIX system call 10 */
extern arg_t _sync(void);         /* FUZIX system call 11 */
extern arg_t _access(void);       /* FUZIX system call 12 */
extern arg_t _chmod(void);        /* FUZIX system call 13 */
extern arg_t _chown(void);        /* FUZIX system call 14 */
extern arg_t _stat(void);         /* FUZIX system call 15 */
extern arg_t _fstat(void);        /* FUZIX system call 16 */
extern arg_t _dup(void);          /* FUZIX system call 17 */
extern arg_t _getpid(void);       /* FUZIX system call 18 */
extern arg_t _getppid(void);      /* FUZIX system call 19 */
extern arg_t _getuid(void);       /* FUZIX system call 20 */
extern arg_t _umask(void);        /* FUZIX system call 21 */
extern arg_t _statfs(void);       /* FUZIX system call 22 */
extern arg_t _execve(void);       /* FUZIX system call 23 */
extern arg_t _getdirent(void);    /* FUZIX system call 24 */
extern arg_t _setuid(void);       /* FUZIX system call 25 */
extern arg_t _setgid(void);       /* FUZIX system call 26 */
extern arg_t _time(void);         /* FUZIX system call 27 */
extern arg_t _stime(void);        /* FUZIX system call 28 */
extern arg_t _ioctl(void);        /* FUZIX system call 29 */
extern arg_t _brk(void);          /* FUZIX system call 30 */
extern arg_t _sbrk(void);         /* FUZIX system call 31 */
extern arg_t _fork(void);         /* FUZIX system call 32 */
extern arg_t _mount(void);        /* FUZIX system call 33 */
extern arg_t _umount(void);       /* FUZIX system call 34 */
extern arg_t _signal(void);       /* FUZIX system call 35 */
extern arg_t _dup2(void);         /* FUZIX system call 36 */
extern arg_t _pause(void);        /* FUZIX system call 37 */
extern arg_t _alarm(void);        /* FUZIX system call 38 */
extern arg_t _kill(void);         /* FUZIX system call 39 */
extern arg_t _pipe(void);         /* FUZIX system call 40 */
extern arg_t _getgid(void);       /* FUZIX system call 41 */
extern arg_t _times(void);        /* FUZIX system call 42 */
extern arg_t _utime(void);        /* FUZIX system call 43 */
extern arg_t _geteuid(void);      /* FUZIX system call 44 */
extern arg_t _getegid(void);      /* FUZIX system call 45 */
extern arg_t _chroot(void);       /* FUZIX system call 46 */
extern arg_t _fcntl(void);        /* FUZIX system call 47 */
extern arg_t _fchdir(void);       /* FUZIX system call 48 */
extern arg_t _fchmod(void);       /* FUZIX system call 49 */
extern arg_t _fchown(void);       /* FUZIX system call 50 */
extern arg_t _mkdir(void);	  /* FUZIX system call 51 */
extern arg_t _rmdir(void);        /* FUZIX system call 52 */
extern arg_t _setpgrp(void);	  /* FUZIX system call 53 */
extern arg_t _uname(void);	  /* FUZIX system call 54 */
extern arg_t _waitpid(void);	  /* FUZIX system call 55 */
extern arg_t _profil(void);	  /* FUZIX system call 56 */
extern arg_t _uadmin(void);	  /* FUZIX system call 57 */
extern arg_t _nice(void);         /* FUZIX system call 58 */
extern arg_t _sigdisp(void);	  /* FUZIX system call 59 */
extern arg_t _flock(void);	  /* FUZIX system call 60 */
extern arg_t _getpgrp(void);	  /* FUZIX system call 61 */
extern arg_t _sched_yield(void);  /* FUZIX system call 62 */
extern arg_t _acct(void);	  /* FUZIX system call 63 */
extern arg_t _memalloc(void);	  /* FUZIX system call 64 */
extern arg_t _memfree(void);	  /* FUZIX system call 65 */

#if defined(CONFIG_32BIT)
#include "kernel32.h"
#endif

#endif /* __FUZIX__KERNEL_DOT_H__ */


