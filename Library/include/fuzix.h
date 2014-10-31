#ifndef _FUZIX_H
#define _FUZIX_H
#include <stdlib.h>
#include <sys/types.h>

/*
 *	FUZIX constants
 */	 

#define __MAXPID 32000
#define NSIGS	16

/* Stat */

#define S_IFMT		0170000
#define S_IFSOCK	0140000		/* Reserved, not used */
#define S_IFLNK		0120000		/* Reserved, not used */
#define S_IFREG		0100000
#define S_IFBLK		0060000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFIFO		0010000

#define S_ISUID		0004000
#define S_ISGID		0002000
#define S_ISVTX		0001000		/* Reserved, not used */
#define S_IRWXU		0000700
#define S_IRUSR		0000400
#define S_IWUSR		0000200
#define S_IXUSR		0000100
#define S_IRWXG		0000070
#define S_IRGRP		0000040
#define S_IWGRP		0000020
#define S_IXGRP		0000010
#define S_IRWXO		0000007
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001

#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define SIGHUP  1       /* hangup */
#define SIGINT  2       /* interrupt */
#define SIGQUIT 3       /* quit */
#define SIGILL  4       /* illegal instruction (not reset when caught */
#define SIGTRAP 5       /* trace trap (not reset when caught) */
#define SIGIOT  6       /* IOT instruction */
#define SIGEMT  7       /* EMT instruction */
#define SIGFPE  8       /* floating point exception */
#define SIGKILL 9       /* kill */
#define SIGBUS  10      /* bus error */
#define SIGSEGV 11      /* segmentation violation */
#define SIGSYS  12      /* bad argument to system call */
#define SIGPIPE 13      /* write on a pipe with no one to read it */
#define SIGALRM 14      /* alarm clock */
#define SIGTERM 15      /* software termination signal from kill */

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

/* waitpid options */
#define WNOHANG		1	/* don't support others yet */


/* Bits 0-7 are saved, bits 8-15 are discard post open. Not all are handled
   in the kernel yet */
#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2
#define O_ACCMODE(x)	((x) & 3)
#define O_APPEND	4
#define O_SYNC		8
#define O_NDELAY	16
#define O_CREAT		256
#define O_EXCL		512
#define O_TRUNC		1024
#define O_NOCTTY	2048
#define O_CLOEXEC	4096

#define F_GETFL		0
#define F_SETFL		1
#define F_GETFD		2
#define F_SETFD		3
#define F_DUPFD		4

#define FNDELAY		O_NDELAY

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

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
#define EWOULDBLOCK	35		/* Operation would block */
#define ENOTEMPTY	36		/* Directory is not empty */
#define ENAMETOOLONG    63              /* File name too long */

/*
 *	FUZIX structures (_xx forms for ones that don't match the POSIX
 *	API versions)
 */

typedef struct {
  uint16_t t_time;
  uint16_t t_data;
} _uzitime_t;

struct _uzitms {
  _uzitime_t tms_utime;
  _uzitime_t tms_stime;
  _uzitime_t tms_cutime;
  _uzitime_t tms_cstime;
  _uzitime_t tms_etime;
};

typedef struct {
  uint16_t o_blkno;
  int16_t o_offset;
} _uzioff_t;

struct  _uzistat
{
	int16_t    st_dev;
	uint16_t   st_ino;
	uint16_t   st_mode;
	uint16_t   st_nlink;
	uint16_t   st_uid;
	uint16_t   st_gid;
	uint16_t   st_rdev;
	_uzioff_t  st_size;
	_uzitime_t st_atime;
	_uzitime_t st_mtime;
	_uzitime_t st_ctime;
};

typedef void (*sighandler_t)(int);
#define  SIG_DFL   (sighandler_t)0
#define  SIG_IGN   (sighandler_t)1

struct _uzisysinfoblk {
  uint8_t infosize;		/* For expandability */
  uint8_t banks;		/* Banks in our 64K (and thus pagesize) */
  uint8_t max_open;
  uint16_t ticks;		/* Tick rate in HZ */
  uint16_t memk;		/* Memory in KB */
  uint16_t usedk;		/* Used memory in KB */
  uint16_t config;		/* Config flag mask */
};

/*
 *	TTY interfaces - may change pending review
 */

struct tty_data {
    char t_ispeed;
    char t_ospeed;
    char t_erase;
    char t_kill;
    int  t_flags;
};

#define TIOCGETP  0
#define TIOCSETP  1
#define TIOCSETN  2
#define TIOCEXCL  3     /** currently not implemented  SN **/
#define UARTSLOW  4     /* Normal interrupt routine (UZI280) */
#define UARTFAST  5     /* Fast interrupt routine for modem usage (UZI280) */
#define TIOCFLUSH 6
#define TIOCGETC  7
#define TIOCSETC  8
              /* UZI280 extensions used by UZI180 in the CP/M 2.2 Emulator */
#define TIOCTLSET 9     /* Don't parse ctrl-chars */
#define TIOCTLRES 10    /* Normal Parse */

#define XTABS   0006000
#define RAW     0000040
#define CRMOD   0000020
#define ECHO    0000010
#define LCASE   0000004
#define CBREAK  0000002
#define COOKED  0000000

/*
 *	Native structures that are actually created by libc not the kernel
 */

typedef unsigned long long time_t;
typedef unsigned long clock_t;
typedef long off_t;
typedef uint16_t nlink_t;
typedef int16_t dev_t;
typedef uint16_t ino_t;
 
struct tms {
  clock_t tms_utime;
  clock_t tms_stime;
  clock_t tms_cutime;
  clock_t tms_cstime;
  clock_t tms_etime;
};

struct stat
{
	dev_t      st_dev;
	ino_t      st_ino;
	mode_t     st_mode;
	nlink_t    st_nlink;
	uid_t      st_uid;
	gid_t      st_gid;
	uint16_t   st_rdev;
	off_t      st_size;
	time_t     st_atime;
	time_t     st_mtime;
	time_t     st_ctime;
};

struct dirent
{
         ino_t	d_ino;
         char	d_name[30];	/* 14 currently used */
};

extern int errno;

extern int _exit(int code);
extern int open(const char *path, int flags, ...);
extern int close(int fd);
extern int creat(const char *path, mode_t mode);
extern int link(const char *path, const char *path2);
extern int unlink(const char *path);
extern int read(int fd, char *buf, int len);
extern int write(int fd, const char *buf, int len);
extern int seek(int fd, int offset, int mode);
extern int chdir(const char *path);
extern int sync(void);
extern int access(const char *path, int way);
extern int chmod(const char *path, mode_t mode);
extern int chown(const char *path, uid_t owner, gid_t group);
extern int dup(int fd);
extern pid_t getpid(void);
extern pid_t getppid(void);
extern uid_t getuid(void);
extern mode_t umask(mode_t);
extern int execve(const char *path, const char *argv[], const char *envp[]);
extern pid_t wait(int *status);
extern int setuid(uid_t uid);
extern int setgid(gid_t gid);
extern int ioctl(int fd, int request,...);
extern int brk(void *addr);
extern void *sbrk(intptr_t increment);
extern pid_t fork(void);
extern int mount(const char *dev, const char *path, int flags);
extern int umount(const char *dev);
extern int signal(int signum, sighandler_t sighandler);
extern int dup2(int oldfd, int newfd);
extern int pause(void);
extern int kill(pid_t pid, int sig);
extern int pipe(int *pipefds);
extern gid_t getgid(void);
extern uid_t geteuid(void);
extern gid_t getegid(void);
extern int chroot(const char *path);
extern int fcntl(int fd, int cmd, ...);
extern int fchdir(int fd);
extern int fchmod(int fd, mode_t mode);
extern int fchown(int fd, uid_t owner, gid_t group);
extern int mkdir(const char *path);
extern int rmdir(const char *path);
extern pid_t setpgrp(void);
extern int waitpid(pid_t pid, int *status, int options);
extern int uadmin(int cmd, int ctrl, void *ptr);
extern int nice(int prio);
extern int rename(const char *path, const char *newpath);

/* asm syscall hooks with C wrappers */
extern int _getdirent(int fd, void *buf, int len);
extern int _stat(const char *path, struct _uzistat *s);
extern int _fstat(int fd, struct _uzistat *s);
extern int _getfsys(int dev, char *buf);
extern int _time(_uzitime_t *t);
extern int _stime(const _uzitime_t *t);
extern int _alarm(int16_t decisecs);
extern int _times(struct _uzitms *t);
extern int _utime(const char *file, _uzitime_t *buf);
extern int _uname(struct _uzisysinfoblk *uzib);
extern int _profil(void *samples, uint16_t offset, uint16_t size, int16_t scale);

/* C library provided syscall emulation */
extern int stat(const char *path, struct stat *s);
extern int fstat(int fd, struct stat *s);
extern int alarm(uint16_t seconds);
extern time_t time(time_t *t);
extern int stime(time_t *t);
extern int times(struct tms *tms);
extern int utime(const char *filename, const struct utimbuf *utim);
extern int uname(struct utsname *buf);
extern int profil(unsigned short *bufbase, size_t bufsize, unsigned long offset,
                  unsigned int scale);

#endif
