#define __UZIFS_DOT_H__

#define ROOTDEV 0
#define ROOTINODE 1
#define SMOUNTED 12742   /* Magic number to specify mounted filesystem */
#define SMOUNTED_WRONGENDIAN 50737   /* byteflipped */
#define CMAGIC   24721
#define UFTSIZE 10
#define NSIGS 16
#define NDEVS 1
#define NBUFS 10
#define OFTSIZE 15
#define ITABSIZE 20
#define _NSIG NSIGS
#define NULLINODE ((inoptr)NULL)
#define NULLBLK ((blkno_t)-1)
#define NULLINOPTR ((inoptr*)NULL)

/* Flags for setftime() */
#define A_TIME 1
#define M_TIME 2
#define C_TIME 4


#define FO_RDONLY        0
#define FO_WRONLY        1
#define FO_RDWR          2

#define ifnot(x) if(!(x))

extern int dev_fd;
extern int dev_offset;
int fd_open(char *name);
int d_close(void);
void xfs_init();
void panic(char *s);
void bufsync (void);
char *zerobuf (void);
int super(void);

extern uint16_t swizzle16(uint32_t v);
extern uint32_t swizzle32(uint32_t v);
extern int swizzling;

typedef struct s_queue {
    char *q_base;    /* Pointer to data */
    char *q_head;    /* Pointer to addr of next char to read. */
    char *q_tail;    /* Pointer to where next char to insert goes. */
    int   q_size;    /* Max size of queue */
    int   q_count;   /* How many characters presently in queue */
    int   q_wakeup;  /* Threshold for waking up processes waiting on queue */
} queue_t;

struct  uzi_stat    /* Really only used by users */
{
    int16_t   st_dev;
    uint16_t  st_ino;
    uint16_t  st_mode;
    uint16_t  st_nlink;
    uint16_t  st_uid;
    uint16_t  st_gid;
    uint16_t  st_rdev;
    uint32_t  st_size;
    uint32_t  fst_atime;
    uint32_t  fst_mtime;
    uint32_t  fst_ctime;
};

typedef struct direct {
        uint16_t   d_ino;
        char     d_name[30];
} direct;

typedef uint16_t blkno_t;    /* Can have 65536 512-byte blocks in filesystem */

typedef struct blkbuf {
    char        bf_data[512];    /* This MUST be first ! */
    char        bf_dev;
    blkno_t     bf_blk;
    char        bf_dirty;
    char        bf_busy;
    uint16_t      bf_time;         /* LRU time stamp */
} blkbuf;

typedef blkbuf *bufptr;

typedef struct dinode {
    uint16_t i_mode;
    uint16_t i_nlink;
    uint16_t i_uid;
    uint16_t i_gid;
    uint32_t    i_size;
    uint32_t   i_atime;
    uint32_t   i_mtime;
    uint32_t   i_ctime;
    blkno_t  i_addr[20];
} dinode;               /* Exactly 64 bytes long! */

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
#define F_BDEV  060000
#define F_CDEV  020000

#define F_MASK  0170000

typedef struct cinode {
    int        c_magic;           /* Used to check for corruption. */
    int        c_dev;             /* Inode's device */
    unsigned   c_num;             /* Inode # */
    dinode     c_node;
    char       c_refs;            /* In-core reference count */
    char       c_dirty;           /* Modified flag. */
} cinode, *inoptr;

typedef struct filesys {
    uint16_t      s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    int16_t       s_nfree;
    blkno_t     s_free[50];
    int16_t       s_ninode;
    uint16_t      s_inode[50];
    uint8_t       s_fmod;
    uint8_t	s_timeh;	/* top bits of time */
    uint32_t      s_time;
    blkno_t     s_tfree;
    uint16_t      s_tinode;
    inoptr      s_mntpt;
} filesys, *fsptr;

typedef struct u_data {
    struct p_tab *u_ptab;       /* Process table pointer */
    char        u_insys;        /* True if in kernel */
    char        u_callno;       /* sys call being executed. */
    char        *u_retloc;      /* Return location from sys call */
    int         u_retval;       /* Return value from sys call */
    int         u_error;                /* Last error number */
    char        *u_sp;          /* Used when a process is swapped. */
    char        *u_bc;          /* Place to save user's frame pointer */
    int         u_cursig;       /* Signal currently being caught */
    int         u_argn;         /* Last system call arg */
    int         u_argn1;        /* This way because args on stack backwards */
    int         u_argn2;
    int         u_argn3;        /* args n-3, n-2, n-1, and n */

    char *      u_base;         /* Source or dest for I/O */
    unsigned    u_count;        /* Amount for I/O */
    uint32_t       u_offset;       /* Place in file for I/O */
    struct blkbuf *u_buf;
    char        u_sysio;        /* True if I/O is to system data space   280*/

    int         u_gid;
    int         u_euid;
    int         u_egid;
    int         u_mask;         /* umask: file creation mode mask */
    uint32_t      u_time;         /* Start time */
    char        u_files[UFTSIZE];       /* Process file table:
                                           contains indexes into open file table. */
    inoptr      u_cwd;          /* Index into inode table of cwd. */
    unsigned    u_break;        /* Top of data space */
    inoptr      u_ino;          /* Used during execve() */
    char        *u_isp;         /* Value of initial sp (argv) */

    int         (*u_sigvec[NSIGS])();   /* Array of signal vectors */
    char        u_name[8];      /* Name invoked with */
    uint32_t      u_utime;        /* Elapsed ticks in user mode */
    uint32_t      u_stime;        /* Ticks in system mode */
    uint32_t      u_cutime;       /* Total childrens ticks */
    uint32_t      u_cstime;
} u_data;


extern inoptr root;
extern struct cinode i_tab[ITABSIZE];
extern struct filesys fs_tab[1];
extern struct blkbuf bufpool[NBUFS];
extern struct u_data udata; /* MUST BE FIRST */
#define PTABSIZE 20
// extern struct p_tab ptab[PTABSIZE];

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

typedef struct oft {
    uint32_t     o_ptr;      /* File position point16_ter */
    inoptr    o_inode;    /* Pointer into in-core inode table */
    char      o_access;   /* O_RDONLY, O_WRONLY, or O_RDWR */
    char      o_refs;     /* Reference count: depends on # of active children*/
} oft;

// extern struct cinode i_tab[ITABSIZE];    /* In-core inode table */
extern struct oft of_tab[OFTSIZE]; /* Open File Table */

void brelse(bufptr bp);
void bawrite(bufptr bp);
int bfree(bufptr bp, int dirty);
int bdwrite(bufptr bp);
int bdread(bufptr bp);
void bufinit(void);
void bufdump(void);
bufptr bfind(int dev, blkno_t blk);
bufptr freebuf(void);
int insq (struct s_queue *q, char c);
int remq (struct s_queue *q, char *cp);
int uninsq (struct s_queue *q, char *cp);
int fullq (struct s_queue *q);
void clrq (struct s_queue *q);
char *bread(int dev, blkno_t blk, int rewrite);
int fmount(int dev, inoptr ino);
void i_ref(inoptr ino);
void xfs_end(void);
int doclose(int16_t uindex);
int _open(char *name, int16_t flag);
int _creat(char *name, int16_t mode);
int _close(int16_t uindex);
int _link( char *name1, char *name2);
int _unlink(char *path);
int _read( int16_t d, char *buf, uint16_t nbytes);
int _write( int16_t d, char *buf, uint16_t nbytes);
int _mknod( char *name, int16_t mode, int16_t dev);
blkno_t bmap(inoptr ip, blkno_t bn, int rwflg);
inoptr rwsetup( int rwflag, int d, char *buf, int nbytes);
int psize(inoptr ino);
void oft_deref(int of);
void i_deref(inoptr ino);
void f_trunc(inoptr ino);
void setftime(inoptr ino, int flag);
void wr_inode(inoptr ino);
int _seek( int16_t file, uint16_t offset, int16_t flag);
void readi( inoptr ino );
void writei( inoptr ino);
void addoff( uint32_t *ofptr, int amount);
void updoff(int d);
inoptr getinode(int uindex);
inoptr n_open( register char *name, register inoptr *parent );
inoptr srch_dir(inoptr wd, register char *compname);
inoptr srch_mt( inoptr ino);
inoptr i_open( register int dev, register unsigned ino);
int ch_link( inoptr wd, char *oldname, char *newname, inoptr nindex);
char * filename( char *path);
int namecomp( char *n1, char *n2);
inoptr newfile( inoptr pino, char *name);
fsptr getdev( int devno);
unsigned i_alloc(int devno);
void i_free( int devno, unsigned ino);
blkno_t blk_alloc( int devno );
void blk_free( int devno, blkno_t blk);
int oft_alloc(void);
void _sync(void);
int _chdir(char *dir);
int min(int a, int b);
int _access( char *path, int16_t mode);
int _chmod( char *path, int16_t mode);
int _chown( char *path, int owner, int group);
int _stat( char *path, struct uzi_stat *buf);
int _fstat( int16_t fd, struct uzi_stat *buf);
void stcpy( inoptr ino, struct uzi_stat *buf);
int _dup( int16_t oldd);
int _dup2( int16_t oldd, int16_t newd);
int _umask( int mask);
int _getfsys(int dev,char * buf);
int _ioctl( int fd, int request, char *data);
int getperm(inoptr ino);
int _mount( char *spec, char *dir, int rwflag);
int _time( int tvec[]);
int getmode(inoptr ino);
void bawrite(bufptr bp);
int isdevice(inoptr ino);
int bfree(bufptr bp, int dirty);
void magic(inoptr ino);
int baddev(fsptr dev);
void validblk(int dev, blkno_t num);
int uf_alloc(void);
void i_ref( inoptr ino);
void freeblk(int dev, blkno_t blk, int level);
int valadr(char *base, uint16_t size);
int _umount(char *spec);

