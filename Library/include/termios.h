#ifndef __TERMIOS_H
#define __TERMIOS_H
#ifndef __TYPES_H
#include <types.h>
#endif

#include <sys/ioctl.h>

typedef uint16_t tcflag_t;
typedef uint16_t speed_t;
typedef uint8_t cc_t;

#define NCCS 12
struct termios {
  tcflag_t c_iflag;
  tcflag_t c_oflag;
  tcflag_t c_cflag;
  tcflag_t c_lflag;
  cc_t c_cc[NCCS];
};

#define VMIN		0	/* Supported */
#define VEOF		0	/* Supported */
#define VTIME		1
#define VEOL		1	/* partial - FIXME, EOF in input */
#define VERASE		2	/* Supported */
#define VINTR		3	/* Supported */
#define VKILL		4	/* Supported */
#define VQUIT		5	/* Supported */
#define VSTART		6	/* Supported */
#define VSTOP		7	/* Supported */
#define VSUSP		8
#define VDSUSP		9
#define VLNEXT		10
#define VDISCARD	11	/* Supported */

#define BRKINT	0x0001
#define ICRNL	0x0002	/* Supported */
#define IGNBRK	0x0004
#define IGNCR	0x0008
#define IGNPAR	0x0010
#define INLCR	0x0020	/* Supported */
#define INPCK	0x0040
#define ISTRIP	0x0080	/* Supported */
#define IUCLC	0x0100
#define IXANY	0x0200
#define IXOFF	0x0400
#define PARMRK	0x0800
#define IXON	0x1000

#define OPOST	0x0001	/* Supported */
#define OLCUC	0x0002
#define ONLCR	0x0004	/* Supported */
#define OCRNL	0x0008
#define ONLRET	0x0010
#define OFILL	0x0020
#define NLDLY	0x0040
#define NL0	0x0000
#define NL1	0x0040
#define CRDLY	0x0180
#define CR0	0x0000
#define CR1	0x0080
#define CR2	0x0100
#define CR3	0x0180
#define TABDLY	0x0600
#define TAB0	0x0000
#define TAB1	0x0200
#define TAB2	0x0400
#define TAB3	0x0600
#define BSDLY	0x0800
#define BS0	0x0000
#define BS1	0x0800
#define VTDLY	0x1000
#define VT0	0x0000
#define VT1	0x1000
#define FFDLY	0x2000
#define FF0	0x0000
#define FF1	0x2000

#define B0	0x0000
#define B50	0x0001
#define B75	0x0002
#define B110	0x0003
#define B134	0x0004
#define B150	0x0005
#define B300	0x0006
#define B600	0x0007
#define B1200	0x0008
#define B2400	0x0009
#define B4800	0x000A
#define B9600   0x000B
#define B19200	0x000C
#define B38400	0x000D
#define B57600	0x000E
#define B115200	0x000F

#define CSIZE	0x0030
#define CS5	0x0000
#define CS6	0x0010
#define CS7	0x0020
#define CS8	0x0030
#define CSTOPB	0x0040
#define CREAD	0x0080
#define PARENB	0x0100
#define PARODD	0x0200
#define HUPCL	0x0400
#define CLOCAL	0x0800
#define CRTSCTS 0x1000
#define CBAUD	0x000F

#define ECHO	0x0001	/* Supported */
#define ECHOE	0x0002	/* Supported */
#define ECHOK	0x0004	/* Supported */
#define ECHONL	0x0008
#define ICANON	0x0010	/* Supported */
#define IEXTEN	0x0020
#define ISIG	0x0040	/* Supported */
#define NOFLSH	0x0080
#define TOSTOP	0x0100
#define XCASE	0x0200

#define _POSIX_VDISABLE	'\0'

#define TCSANOW		0
#define TCSADRAIN	1
#define TCSAFLUSH	2

#define TCIFLUSH	1
#define TCOFLUSH	2
#define TCIOFLUSH	3

#define TCIOFF		0
#define TCION		1
#define TCOOFF		2
#define TCOON		3

#define TCGETS		1
#define TCSETS		2
#define TCSETSW		3
#define TCSETSF		4
#define TIOCINQ		5
#define TIOCFLUSH	6
#define TIOCHANGUP	7	/* vhangup() */
#define TIOCOSTOP	8
#define TIOCOSTART	9
#define TIOCGWINSZ	10
#define TIOCSWINSZ	11
#define TIOCGPGRP	12
#define TIOCSPGRP	13

#define KBMAPSIZE	0x20
#define KBMAPGET	0x21
#define VTSIZE		0x22
#define KBSETTRANS	(0x23|__IOCTL_SUPER)
#define VTATTRS		0x24
#define KBRATE		0x25

#define VTFONTINFO	0x30
#define VTSETFONT	(0x31|__IOCTL_SUPER)
#define VTGETFONT	0x32
#define VTSETUDG	0x33
#define VTGETUDG	0x34

#define VTBORDER	0x35
#define VTINK		0x36
#define VTPAPER		0x37

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

extern speed_t cfgetispeed(const struct termios *__termios_p);
extern speed_t cfgetospeed(const struct termios *__termios_p);
extern int cfsetispeed(struct termios *__termios_p, speed_t __speed);
extern int cfsetospeed(struct termios *__termios_p, speed_t __speed);

extern void cfmakeraw (struct termios *__t);

extern int tcsetattr(int __fd, int __opt, const struct termios *__termios_p);
extern int tcgetattr(int __fildes, struct termios *__termios_p);
extern int tcdrain(int __fildes);
extern int tcflow(int __fildes, int __action);
extern int tcflush(int __fildes, int __queue_selector);
extern int tcsendbreak(int __fildes, int __duration);
extern int tcgetpgrp(int __fildes);
extern int tcsetpgrp(int __fildes, int __pgrp_id);

struct vt_repeat {
  uint8_t first;
  uint8_t continual;
};

#endif
