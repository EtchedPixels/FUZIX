#ifndef __TTY_DOT_H__
#define __TTY_DOT_H__

typedef uint16_t tcflag_t;
typedef uint16_t speed_t;
typedef uint8_t cc_t;

#define NCCS 12
struct termios {
  tcflag_t c_iflag;
  tcflag_t c_oflag;
  tcflag_t c_cflag;
  tcflag_t c_lflag;	/* 8 bytes */
  cc_t c_cc[NCCS];	/* + 12 -> 20 total */
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

#define KBMAPSIZE	0x20
#define KBMAPGET	0x21
#define VTSIZE		0x22
#define KBSETTRANS	0x23

/* Character Input Queue size */
#define TTYSIZ 132

/* Group the tty into a single object. That lets 8bit processors keep all
   the data indexed off a single register */
struct tty {
    /* Put flag first: makes it cheaper when short of registers */
    uint8_t flag;		/* Use uint8 pad - makes the whole struct
                                   24 byte - a nice number for CPUs with no 
                                   multiplier */
    uint8_t pad0;
#define TTYF_STOP	1
#define TTYF_DISCARD	2
#define TTYF_DEAD	4
    uint16_t pgrp;
    struct termios termios;
};

extern struct tty ttydata[NUM_DEV_TTY + 1];

extern CODE1 void tty_init(void);

extern CODE1 int tty_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern CODE1 int tty_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern CODE1 int tty_open(uint8_t minor, uint16_t flag);
extern CODE1 int tty_close(uint8_t minor);
extern CODE1 int tty_ioctl(uint8_t minor, uint16_t request, char *data);

extern CODE1 void tty_hangup(uint8_t minor);
extern CODE1 void tty_carrier_drop(uint8_t minor);
extern CODE1 void tty_carrier_raise(uint8_t minor);

extern CODE1 int ptty_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern CODE1 int ptty_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern CODE1 int ptty_open(uint8_t minor, uint16_t flag);
extern CODE1 int ptty_close(uint8_t minor);
extern CODE1 int ptty_ioctl(uint8_t minor, uint16_t request, char *data);

extern CODE1 int pty_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern CODE1 int pty_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern CODE1 int pty_open(uint8_t minor, uint16_t flag);
extern CODE1 int pty_close(uint8_t minor);
extern CODE1 int pty_ioctl(uint8_t minor, uint16_t request, char *data);

extern CODE1 int tty_inproc(uint8_t minor, unsigned char c);
extern CODE1 void tty_outproc(uint8_t minor);
extern CODE1 void tty_echo(uint8_t minor, unsigned char c);
extern CODE1 void tty_erase(uint8_t minor);
extern CODE1 void tty_putc_wait(uint8_t minor, unsigned char c);

/* provided by platform */
extern struct s_queue ttyinq[NUM_DEV_TTY + 1];
extern CODE2 bool tty_writeready(uint8_t minor);
extern CODE2 void tty_putc(uint8_t minor, unsigned char c);
extern CODE2 void tty_setup(uint8_t minor);
extern CODE2 int tty_carrier(uint8_t minor);
/* PTY pieces: 8 ptys both sides of */
#ifdef CONFIG_PTY_DEV
#define PTY_BUFFERS \
static char pbuf0[TTYSIZ];\
static char pbuf1[TTYSIZ];\
static char pbuf2[TTYSIZ];\
static char pbuf3[TTYSIZ];\
static char pbuf4[TTYSIZ];\
static char pbuf5[TTYSIZ];\
static char pbuf6[TTYSIZ];\
static char pbuf7[TTYSIZ];\
static char pbuf8[TTYSIZ];\
static char pbuf9[TTYSIZ];\
static char pbufa[TTYSIZ];\
static char pbufb[TTYSIZ];\
static char pbufc[TTYSIZ];\
static char pbufd[TTYSIZ];\
static char pbufe[TTYSIZ];\
static char pbuff[TTYSIZ];\

#define PTY_QUEUES \
    {pbuf0, pubf0, pubf0, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf1, pubf1, pubf1, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf2, pubf2, pubf2, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf3, pubf3, pubf3, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf4, pubf4, pubf4, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf5, pubf5, pubf5, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf6, pubf6, pubf6, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf7, pubf7, pubf7, TTYSIZ, 0, TTYSIZ/2}
    {pbuf8, pubf8, pubf8, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf9, pubf9, pubf9, TTYSIZ, 0, TTYSIZ/2}, \
    {pbufa, pubfa, pubfa, TTYSIZ, 0, TTYSIZ/2}, \
    {pbufb, pubfb, pubfb, TTYSIZ, 0, TTYSIZ/2}, \
    {pbufc, pubfc, pubfc, TTYSIZ, 0, TTYSIZ/2}, \
    {pbufd, pubfd, pubfd, TTYSIZ, 0, TTYSIZ/2}, \
    {pbufe, pubfe, pubfe, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuff, pubff, pubff, TTYSIZ, 0, TTYSIZ/2}

#else

#define PTY_BUFFERS
#define PTY_QUEUES
#define PTY_OFFSET	NUM_DEV_TTY
#endif /* CONFIG_PTY_DEV */
#endif /* TTY_DOT_H */
