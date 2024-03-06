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
#define VTIME		1	/* Supported */
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

#define BRKINT	0x0001	/* Supported on level 2 */
#define ICRNL	0x0002	/* Supported */
#define IGNBRK	0x0004	/* Supported on level 2 */
#define IGNCR	0x0008  /* Supported */
#define IGNPAR	0x0010	/* Supported on level 2 */
#define INLCR	0x0020	/* Supported */
#define INPCK	0x0040	/* Supported on level 2 */
#define ISTRIP	0x0080	/* Supported */
#define IUCLC	0x0100	/* Not POSIX */
#define IXANY	0x0200
#define IXOFF	0x0400
#define PARMRK	0x0800	/* Supported on level 2 */
#define IXON	0x1000

#define _ISYS	(IGNCR|ICRNL|INLCR|ISTRIP)	/* Flags supported by core */

#define OPOST	0x0001	/* Supported */
#define OLCUC	0x0002	/* Not POSIX */
#define ONLCR	0x0004	/* Supported */
#define OCRNL	0x0008	/* Supported */
#define ONLRET	0x0010
#define OFILL	0x0020	/* Delays are not supported */
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

#define _OSYS	(OPOST|ONLCR|OCRNL)

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

#define CSIZE	0x0030	/* Supported */
#define CS5	0x0000
#define CS6	0x0010
#define CS7	0x0020
#define CS8	0x0030
#define CSTOPB	0x0040	/* Supported */
#define CREAD	0x0080
#define PARENB	0x0100	/* Supported for level 2 input */
#define PARODD	0x0200	/* Supported */
#define HUPCL	0x0400	/* Supported */
#define CLOCAL	0x0800	/* Supported */
#define CRTSCTS 0x1000	/* Supported (only a few uart drivers) */
#define CMSPAR	0x2000	/* Supported for level 2 input */
#define CBAUD	0x000F	/* Supported */

#define _CSYS	(CREAD|HUPCL|CLOCAL)

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

#define _LSYS	(ECHO|ECHOE|ECHOK|ICANON|ISIG)

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
#define KBSETTRANS	(0x23|IOCTL_SUPER)
#define VTATTRS		0x24
#define KBRATE		0x25

#define VTFONTINFO	0x30
#define VTSETFONT	(0x31|IOCTL_SUPER)
#define VTGETFONT	0x32
#define VTSETUDG	0x33
#define VTGETUDG	0x34
#define VTBORDER	0x35
#define VTINK		0x36
#define VTPAPER		0x37

/* Fuzix systems to level 2 have 256 byte tty buffers as per standards, level 1
   boxes may not */
#if defined(CONFIG_LEVEL_2)
#define TTYSIZ 256
#endif

/* Character Input Queue size */
#if !defined TTYSIZ
#define TTYSIZ 132
#endif

struct winsize {		/* Keep me 8bytes on small boxes */
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

struct fontinfo {
    uint8_t font_low;
    uint8_t font_high;
    uint8_t udg_low;
    uint8_t udg_high;
    uint8_t format;
#define FONT_INFO_8X8	0
#define FONT_INFO_6X8	1
#define FONT_INFO_4X8	2	/* packed twice in each byte */
#define FONT_INFO_4X6	3
#define FONT_INFO_8X11P16  4	/* 8 x 11 but packed 16 line packed */
#define FONT_INFO_8X16	5
#define FONT_INFO_6X12P16  6	/* 6x12 on 16 byte boundaries
				   16 line packed, low 6 bits */
#define FONT_INFO_8X10P16  7	/* 8 x 10 but packed 16 line packed */
};

/* Group the tty into a single object. That lets 8bit processors keep all
   the data indexed off a single register */
struct tty {
    /* Put flag first: makes it cheaper when short of registers */
    uint8_t flag;		/* make the whole struct
                                   32 byte - a nice number for CPUs with no 
                                   multiplier */
    uint8_t users;
#define TTYF_STOP	1
#define TTYF_DISCARD	2
#define TTYF_DEAD	4
 /* FIXME add TTYF_SLEEPING 8 here so can generalize code */
    uint16_t pgrp;
    struct termios termios;
    struct winsize winsize;	/* 8 byte so takes us up to 32 */
};

#define CTRL(x)		((x)&0x1F)

extern struct tty ttydata[NUM_DEV_TTY + 1];
extern tcflag_t termios_mask[NUM_DEV_TTY + 1];

extern void tty_init(void);

extern int tty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int tty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int tty_open(uint_fast8_t minor, uint16_t flag);
extern int tty_close(uint_fast8_t minor);
extern int tty_ioctl(uint_fast8_t minor, uarg_t request, char *data);

#define tty_pending(minor) ttyinq[(uint8_t)(minor)].q_count

extern void tty_exit(void);
extern void tty_post(inoptr ino, uint_fast8_t minor, uint16_t flag);

extern void tty_hangup(uint_fast8_t minor);
extern void tty_carrier_drop(uint_fast8_t minor);
extern void tty_carrier_raise(uint_fast8_t minor);

extern int ptty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int ptty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int ptty_open(uint_fast8_t minor, uint16_t flag);
extern int ptty_close(uint_fast8_t minor);
extern int ptty_ioctl(uint_fast8_t minor, uint16_t request, char *data);

extern int pty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int pty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int pty_open(uint_fast8_t minor, uint16_t flag);
extern int pty_close(uint_fast8_t minor);
extern int pty_ioctl(uint_fast8_t minor, uint16_t request, char *data);

extern uint_fast8_t tty_inproc(uint_fast8_t minor, uint_fast8_t c);
extern void tty_outproc(uint_fast8_t minor);
extern void tty_echo(uint_fast8_t minor, uint_fast8_t c);
extern void tty_erase(uint_fast8_t minor);
extern uint_fast8_t tty_putc_maywait(uint_fast8_t minor, uint_fast8_t, uint_fast8_t flags);
extern void tty_putc_wait(uint_fast8_t minor, uint_fast8_t c);

/* Level 2 helpers */
extern int tty_inproc_bad(uint_fast8_t minor, uint_fast8_t ch);
extern int tty_inproc_full(uint_fast8_t minor, uint_fast8_t ch);
extern int tty_inproc_softparity(uint_fast8_t minor, uint_fast8_t ch);
extern void tty_break_event(uint_fast8_t minor);
extern uint8_t tty_add_parity(uint_fast8_t minor, uint8_t ch);

typedef enum {
    TTY_READY_NOW=1,    /* port is ready immediately */
    TTY_READY_SOON=0,   /* we'll be ready shortly, kernel should spin, polling the port repeatedly */
    TTY_READY_LATER=2  /* we'll be a long time, put this process to sleep and schedule another */
} ttyready_t;

/* provided by platform */
extern struct s_queue ttyinq[NUM_DEV_TTY + 1];
extern ttyready_t tty_writeready(uint_fast8_t minor);
extern void tty_sleeping(uint_fast8_t minor);
extern void tty_putc(uint_fast8_t minor, uint_fast8_t c);
extern void tty_setup(uint_fast8_t minor, uint_fast8_t flags);
extern int tty_carrier(uint_fast8_t minor);
extern void tty_data_consumed(uint_fast8_t minor);
/* PTY pieces: 8 ptys both sides of */
#ifdef CONFIG_PTY_DEV
#define PTY_BUFFERS \
static uint8_t pbuf0[TTYSIZ];\
static uint8_t pbuf1[TTYSIZ];\
static uint8_t pbuf2[TTYSIZ];\
static uint8_t pbuf3[TTYSIZ];\
static uint8_t pbuf4[TTYSIZ];\
static uint8_t pbuf5[TTYSIZ];\
static uint8_t pbuf6[TTYSIZ];\
static uint8_t pbuf7[TTYSIZ];\
static uint8_t pbuf8[TTYSIZ];\
static uint8_t pbuf9[TTYSIZ];\
static uint8_t pbufa[TTYSIZ];\
static uint8_t pbufb[TTYSIZ];\
static uint8_t pbufc[TTYSIZ];\
static uint8_t pbufd[TTYSIZ];\
static uint8_t pbufe[TTYSIZ];\
static uint8_t pbuff[TTYSIZ];\

#define PTY_QUEUES \
    {pbuf0, pubf0, pubf0, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf1, pubf1, pubf1, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf2, pubf2, pubf2, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf3, pubf3, pubf3, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf4, pubf4, pubf4, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf5, pubf5, pubf5, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf6, pubf6, pubf6, TTYSIZ, 0, TTYSIZ/2}, \
    {pbuf7, pubf7, pubf7, TTYSIZ, 0, TTYSIZ/2}, \
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
