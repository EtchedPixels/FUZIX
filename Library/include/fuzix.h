#ifndef _FUZIX_H
#define _FUZIX_H
#include <stdlib.h>
#include <sys/types.h>

/*
 *	FUZIX constants
 */	 

#define __MAXPID 32000

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
 
struct dirent
{
         ino_t	d_ino;
         char	d_name[30];	/* 14 currently used */
};


#endif
