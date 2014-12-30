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
