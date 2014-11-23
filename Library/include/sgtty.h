#ifndef _SGTTY_H
#define _SGTTY_H

/* FIXME: should go away to be replaced with termios */
#define TIOCGETP 0
#define TIOCSETP 1
#define TIOCSETN  2
/**
#define TIOCEXCL  3
#define TIOCNXCL  4
#define TIOCHPCL  5
**/
#define TIOCFLUSH 6
#define TIOCGETC  7
#define TIOCSETC  8
#define TIOCTLSET 9
#define TIOCTLRES 10

#define XTABS	0006000
#define RAW	0000040
#define CRMOD   0000020
#define CRLF	0000020
#define ECHO	0000010
#define LCASE	0000004
#define CBREAK	0000002
#define COOKED  0000000


struct sgttyb {
    char sg_ispeed, sg_ospeed;
    char sg_erase, sg_kill;
    int sg_flags;
};

struct tchars {
    char	t_intrc,t_quit,t_start,t_stop,t_eof;
};

#define stty( fd, s)	(ioctl(fd, TIOCSETP, s))
#define gtty( fd, s)	(ioctl(fd, TIOCGETP, s))

#endif
