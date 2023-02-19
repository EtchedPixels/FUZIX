#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <graphics.h>
#include <devtty.h>
#include <rcbus.h>
#include <vt.h>
#include <cpu_ioctl.h>
#include <softzx81.h>
#include "z180_uart.h"

struct uart *uart[NUM_DEV_TTY + 1];
uint8_t nuart = 1;
static uint8_t first_poll = 1;
uint16_t ttyport[NUM_DEV_TTY + 1];
static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1];

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS
};

uint8_t vidmode;	/* For live screen */
static uint8_t mode[5];	/* Per console 1-4 */
static uint8_t tmsinkpaper[5] = {0, 0xF4, 0xF4, 0xF4, 0xF4 };
static uint8_t tmsborder[5] = { 0, 0x04, 0x04, 0x04, 0x04 };
static uint8_t efinkpaper[5] = { 0, 0x8F, 0x8F, 0x8F, 0x8F };
static uint8_t vswitch;	/* Track vt switch locking due top graphics maps */
uint8_t vt_twidth;
uint8_t vt_tright;

/* FIXME: CBAUD general handling - may need 0.4 changes to fix */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	used(flags);
	uart[minor]->setup(minor);
}

int tty_carrier(uint_fast8_t minor)
{
	return uart[minor]->carrier(minor);
}

/* Poll each present UART. The helper returns a value to add which means
   we can do
   0 : call me again
   1 : normal
   2 : skip next port (used with SIO)
   4 : skip 3 more ports (used with QUART)

   The rx side wants moving to proper interrupt driven queues */

void tty_pollirq(void)
{
	uint8_t minor = first_poll;
	while(minor < nuart)
		minor += uart[minor]->intr(minor);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
        /* If we have a video display then the first console output will
	   go to it as well *unless it has a keyboard too* */
        if (minor == 1 && shadowcon && !vswitch)
		vtoutput(&c, 1);
        uart[minor]->putc(minor, c);
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return uart[minor]->writeready(minor);
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
	/* Our console is minor 1 */
	if (c == '\n')
		kputchar('\r');
	while(tty_writeready(1) != TTY_READY_NOW);
	tty_putc(1, c);
}

int rctty_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor <= NUM_DEV_TTY && ttyport[minor])
		return tty_open(minor, flag);
	udata.u_error = ENXIO;
	return -1;
}

/*
 *	Actual UART objects
 */

static uint8_t acia_intr(uint8_t minor)
{
	uint8_t ca = ACIA_C;

	if (ca & 1)
		tty_inproc(minor, ACIA_D);
	return 1;
}

static void acia_setup(uint8_t minor)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r = t->c_cflag & CSIZE;

	used(minor);

	/* No CS5/CS6 CS7 must have parity enabled */
	if (r <= CS7) {
		t->c_cflag &= ~CSIZE;
		t->c_cflag |= CS7|PARENB;
	}
	/* No CS8 parity and 2 stop bits */
	if (r == CS8 && (t->c_cflag & PARENB))
		t->c_cflag &= ~CSTOPB;
	/* There is no obvious logic to this */
	switch(t->c_cflag & (CSIZE|PARENB|PARODD|CSTOPB)) {
	case CS7|PARENB:
		r = 0x8A;
		break;
	case CS7|PARENB|PARODD:
		r = 0x8E;
		break;
	case CS7|PARENB|CSTOPB:
		r = 0x82;
	case CS7|PARENB|PARODD|CSTOPB:
		r = 0x86;
	case CS8|CSTOPB:
		r = 0x92;
		break;
	default:
	case CS8:
		r = 0x96;
		break;
	case CS8|PARENB:
		r = 0x9A;
		break;
	case CS8|PARENB|PARODD:
		r = 0x9E;
		break;
	}
	ACIA_C = r;
}

static uint8_t acia_writeready(uint_fast8_t minor)
{
	used(minor);

	if (ACIA_C & 0x02)	/* THRE? */
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

static void acia_putc(uint_fast8_t minor, uint_fast8_t c)
{
	used(minor);
	ACIA_D = c;
}

static uint8_t carrier_unwired(uint_fast8_t minor)
{
	used(minor);
	return 1;
}


struct uart acia_uart = {
	acia_intr,
	acia_writeready,
	acia_putc,
	acia_setup,
	carrier_unwired,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	"ACIA"
};

static uint8_t old_c[NUM_DEV_TTY + 1];

static uint8_t sio_intrb(uint_fast8_t minor)
{
	uint8_t r;
	uint8_t p = ttyport[minor];
	r = in(p);
	if (r & 1)
		tty_inproc(minor, in(p + 1));
	if (r & 2)
		out(p, 2 << 5);
	if ((r ^ old_c[minor]) & 8) {
		old_c[minor] = r;
		if (r & 8)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	return 1;
}

static uint8_t sio_intr(uint_fast8_t minor)
{
	uint8_t r;
	uint8_t p = ttyport[minor];
	r = in(p);
	if (!(r & 2))
		return 2;
	return sio_intrb(minor);
}

/* Be careful here. We need to peek at RR but we must be sure nobody else
   interrupts as we do this. Really we want to switch to irq driven tx ints
   on this platform I think. Need to time it and see

   An asm common level tty driver might be a better idea

   Need to review this we should be ok as the IRQ handler always leaves
   us pointing at RR0 */
static ttyready_t sio_writeready(uint_fast8_t minor)
{
	irqflags_t irq;
	uint8_t c;

	uint8_t p = ttyport[minor];

	irq = di();
	out(p, 0);
	c = in(p);
	irqrestore(irq);

	if (c & 0x04)	/* THRE? */
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}


static void sio_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t p = ttyport[minor];
	out(p + 1, c);
}

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
	0x05, 0xEA
};

static uint8_t siobaud[] = {
	0xC0,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0,	/* 134 */
	0,	/* 150 */
	/* x 64 clock */
	0xC0,	/* 300 */
	0x60,	/* 600 */
	0x30,	/* 1200 */
	0x18,	/* 2400 */
	0x0C,	/* 4800 */
	0x06,	/* 9600 */
	0x03,	/* 19200 */
	/* x 32 clock */
	0x03,	/* 38400 */
	0x02,	/* 57600 */
	0x01	/* 115200 */
};

/* Note: in theory we can support below 300 baud in some cases but it's
   not useful so we don't bother right now */

static uint8_t siobaud2[] = {
	0x00,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0,	/* 134 */
	0,	/* 150 */
	0x17,	/* 300 */
	0x16,	/* 600 */
	0x15,	/* 1200 */
	0x14,	/* 2400 */
	0x13,	/* 4800 */
	0x12,	/* 9600 */
	0x11,	/* 19200 */
	0x10,	/* 38400 */
	0x01,	/* 57600 */
	0x00	/* 115200 */
};

__sfr __at 0xED z512_ctl;

static void sio_do_setup(uint_fast8_t minor)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;
	uint8_t p = ttyport[minor];

	baud = t->c_cflag & CBAUD;
	if (baud < B300)
		baud = B300;

	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = 0xC4;
	if (ctc_port && p == SIOB_C) {
		out(ctc_port + 1, 0x55);
		out(ctc_port + 1, siobaud[baud]);
		if (baud > B19200)	/* Use x32 clock and CTC divider */
			r = 0x84;
	} else if (z512_present && p == SIOB_C)
		z512_ctl = siobaud2[baud];
	else
		baud = B115200;

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;

	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->c_cflag & CSIZE) << 1);
}

static void sio_setup(uint_fast8_t minor)
{
	uint8_t p = ttyport[minor];
	sio_do_setup(minor);
	sio2_otir(p);
}

static uint8_t sio_carrier(uint_fast8_t minor)
{
        uint8_t c;
	uint8_t p = ttyport[minor];

	out(p, 0);
	c = in(p);
	if (c & 0x08)
		return 1;
	return 0;
}

struct uart sio_uart = {
	sio_intr,
	sio_writeready,
	sio_putc,
	sio_setup,
	sio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"Z80 SIO"
};

struct uart sio_uartb = {
	sio_intrb,
	sio_writeready,
	sio_putc,
	sio_setup,
	sio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"Z80 SIO"
};

/*
 *	Annoyingly the RC2014 SIO has the low bit reversed which means
 *	the KIO (which is the right way around) needs its own entries
 */
static uint8_t kio_intrb(uint_fast8_t minor)
{
	uint8_t r;
	uint8_t p = ttyport[minor];
	r = in(p + 1);
	if (r & 1)
		tty_inproc(minor, in(p));
	if (r & 2)
		out(p + 1, 2 << 5);
	if ((r ^ old_c[minor]) & 8) {
		old_c[minor] = r;
		if (r & 8)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	return 1;
}

static uint8_t kio_intr(uint_fast8_t minor)
{
	uint8_t r;
	uint8_t p = ttyport[minor];
	r = in(p + 1);
	if (!(r & 2))
		return 2;
	return kio_intrb(minor);
}

/* Be careful here. We need to peek at RR but we must be sure nobody else
   interrupts as we do this. Really we want to switch to irq driven tx ints
   on this platform I think. Need to time it and see

   An asm common level tty driver might be a better idea

   Need to review this we should be ok as the IRQ handler always leaves
   us pointing at RR0 */
static ttyready_t kio_writeready(uint_fast8_t minor)
{
	irqflags_t irq;
	uint8_t c;

	uint8_t p = ttyport[minor];

	irq = di();
	out(p + 1, 0);
	c = in(p + 1);
	irqrestore(irq);

	if (c & 0x04)	/* THRE? */
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}


static void kio_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t p = ttyport[minor];
	out(p, c);
}

static void kio_setup(uint_fast8_t minor)
{
	uint8_t p = ttyport[minor];
	sio_do_setup(minor);
	sio2_otir(p + 1);
}


static uint8_t kio_carrier(uint_fast8_t minor)
{
        uint8_t c;
	uint8_t p = ttyport[minor];

	out(p + 1, 0);
	c = in(p + 1);
	if (c & 0x08)
		return 1;
	return 0;
}

struct uart kio_uart = {
	kio_intr,
	kio_writeready,
	kio_putc,
	kio_setup,
	kio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"Z80 KIO"
};

struct uart kio_uartb = {
	kio_intrb,
	kio_writeready,
	kio_putc,
	kio_setup,
	kio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"Z80 KIO"
};

/* We are clocking at 1.8432MHz and x16 or x64 for the low speeds */
static uint16_t eipc_siobaud[] = {
	0xC0,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0xD6,	/* 134 */
	0xC0,	/* 150 */
	0x60,	/* 300 */
	0xC0,	/* 600 */
	0x60,	/* 1200 */
	0x30,	/* 2400 */
	0x18,	/* 4800 */
	0x0C,	/* 9600 */
	0x06,	/* 19200 */
	0x03,	/* 38400 */
	0x02,	/* 57600 */
	0x01	/* 115200 */
};

static void eipc_setup(uint8_t minor)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;

	baud = t->c_cflag & CBAUD;
	if (baud < B134)
		baud = B134;

	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = 0xC4;

	out(ctc_port + minor, 0x55);
	out(ctc_port + minor,  siobaud[baud]);

	if (baud >= B600)	/* Use x16 clock and CTC divider */
		r = 0x44;

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;

	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->c_cflag & CSIZE) << 1);
	sio2_otir(ttyport[minor]);
}

struct uart eipc_uart = {
	kio_intr,
	kio_writeready,
	kio_putc,
	kio_setup,
	kio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"Z80 EIPC"
};

struct uart eipc_uartb = {
	kio_intrb,
	kio_writeready,
	kio_putc,
	eipc_setup,
	kio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"Z80 EIPC"
};

/* Discrete SIO mapped and clocked the same way
   as on the EIPC */
struct uart easy_uart = {
	kio_intr,
	kio_writeready,
	kio_putc,
	kio_setup,
	kio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"SIO"
};

struct uart easy_uartb = {
	kio_intrb,
	kio_writeready,
	kio_putc,
	eipc_setup,
	kio_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"SIO"
};


static uint8_t ns16x50_intr(uint_fast8_t minor)
{
	uint8_t msr;
	uint8_t port = ttyport[minor];

	if (in(port + 5) & 1)
		tty_inproc(minor, in(port));
	msr = in(port + 6);
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	/* TODO: CTS/RTS */
	return 1;
}

static ttyready_t ns16x50_writeready(uint_fast8_t minor)
{
	uint8_t port = ttyport[minor];
	uint8_t n = in(port + 5);
	used(minor);
	return n & 0x20 ? TTY_READY_NOW : TTY_READY_SOON;
}

static void ns16x50_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t port = ttyport[minor];
	out(port, c);
}

/*
 *	16x50 conversion betwen a Bxxxx speed rate (see tty.h) and the values
 *	to stuff into the chip.
 */
static uint16_t clocks[] = {
	12,		/* Not a real rate */
	2304,
	1536,
	1047,
	857,
	768,
	384,
	192,
	96,
	48,
	24,
	12,
	6,
	3,
	2,
	1
};


static void ns16x50_setup(uint_fast8_t minor)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;
	uint8_t port = ttyport[minor];

	d = 0x80;	/* DLAB (so we can write the speed) */
	d |= (t->c_cflag & CSIZE) >> 4;
	if(t->c_cflag & CSTOPB)
		d |= 0x04;
	if (t->c_cflag & PARENB)
		d |= 0x08;
	if (!(t->c_cflag & PARODD))
		d |= 0x10;
	out(port + 3, d);	/* LCR */
	w = clocks[t->c_cflag & CBAUD];
	out(port, w);		/* Set the DL */
	out(port + 1, w >> 8);
	if (w >> 8)	/* Low speeds interrupt every byte for latency */
		out(port + 2, 0x00);
	else		/* High speeds set our interrupt quite early
			   as our latency is poor, turn on 64 byte if
			   we have a 16C750 */
		out(port + 2, 0x51);
	out(port + 3, d & 0x7F);
	/* FIXME: CTS/RTS support */
	out(port + 4, 0x03); /* DTR RTS */
	out(port + 1, 0x0D); /* We don't use tx ints */
}

static uint8_t ns16x50_carrier(uint_fast8_t minor)
{
	uint8_t port = ttyport[minor];
	return in(port + 6) & 0x80;
}

struct uart ns16x50_uart = {
	ns16x50_intr,
	ns16x50_writeready,
	ns16x50_putc,
	ns16x50_setup,
	ns16x50_carrier,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	"16x50"
};

static uint8_t asci_intr0(uint_fast8_t minor)
{
	while(ASCI_STAT0 & 0x80)
		tty_inproc(minor, ASCI_RDR0);
	return 1;
}

static uint8_t asci_intr1(uint_fast8_t minor)
{
	while(ASCI_STAT1 & 0x80)
		tty_inproc(minor, ASCI_RDR1);
	return 1;
}

ttyready_t asci_writeready0(uint_fast8_t minor)
{
	used(minor);
        if (ASCI_STAT0 & 0x02)
	        return TTY_READY_NOW;
	return TTY_READY_SOON;
}

ttyready_t asci_writeready1(uint_fast8_t minor)
{
	used(minor);
        if (ASCI_STAT1 & 0x02)
	        return TTY_READY_NOW;
	return TTY_READY_SOON;
}


static void asci_putc0(uint_fast8_t minor, uint_fast8_t c)
{
	used(minor);
	while(!(ASCI_STAT0 & 2));
	ASCI_TDR0 = c;
}

static void asci_putc1(uint_fast8_t minor, uint_fast8_t c)
{
	used(minor);
	while(!(ASCI_STAT1 & 2));
	ASCI_TDR1 = c;
}

/* bit 5: turn on divide by 30 v 10
   bit 3: turn on scale by 64 not 16
   bit 2-0: 2^n for scaling (not 111) */
static const uint8_t baudtable[] = {
    /* Dividers for our clock. Table is smaller than the maths by far */
    0,
    0,		/* 50 */
    0,		/* 75 */
    0,		/* 110 */
    0,  	/* 134.5 */
    0x2E,	/* 150 */
    0x2D,	/* 300 */
    0x2C,	/* 600 */
    0x2B,	/* 1200 */
    0x2A,	/* 2400 */
    0x29,	/* 4800 */
    0x28,	/* 9600 */
    /* Now switch to 16x clock */
    0x21,	/* 19200 */
    0x20,	/* 38400 */
    /* And 10x scaler */
    0x01,	/* 57600 */
    0x00,	/* 115200 */
};

static void asci_setup(uint_fast8_t minor)
{
    struct termios *t = &ttydata[minor].termios;
    uint8_t cntla = 0x60;
    uint8_t cntlb = 0;
    uint16_t cflag = t->c_cflag;
    uint8_t baud;
    uint8_t ecr = 0;
    uint8_t port = ttyport[minor];

    /* Calculate the control bits */
    if (cflag & PARENB) {
        cntla |= 2;
        if (cflag & PARODD)
            cntlb |= 0x10;
    }
    if ((cflag & CSIZE) == CS8)
        cntla |= 4;
    else {
        cflag &= ~CSIZE;
        cflag |= CS7;
    }
    if (cflag & CSTOPB)
        cntla |= 1;

    /* Handle the baud table. Right now this is hardcoded for our clock */

    baud = cflag & CBAUD;
    /* We can't get below 150 easily. We might be able to do this with the
       BRG on one channel - need to check FIXME */
    if (baud && baud < B150) {
        baud = B150;
        cflag &= ~CBAUD;
        cflag |= B150;
    }
    cntlb |= baudtable[baud];

    if (port & 2) {
        if (cflag & CRTSCTS)
            ecr = 0x20;
        /* FIXME: need to do software RTS side */
    } else {
        cflag &= ~CRTSCTS;
    }

    t->c_cflag = cflag;

    /* ASCI serial set up */
   if ((port & 2) == 0) {
        ASCI_CNTLA0 = cntla;
        ASCI_CNTLB0 = cntlb;
        ASCI_ASEXT0 &= ~0x20;
        ASCI_ASEXT1 |= ecr;
    } else {
        ASCI_CNTLA1 = cntla;
        ASCI_CNTLB1 = cntlb;
    }
}

struct uart z180_uart0 = {
	asci_intr0,
	asci_writeready0,
	asci_putc0,
	asci_setup,
	carrier_unwired,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|CRTSCTS|_CSYS,
	"ASCI UART",
};

struct uart z180_uart1 = {
	asci_intr1,
	asci_writeready1,
	asci_putc1,
	asci_setup,
	carrier_unwired,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|CRTSCTS|_CSYS,
	"ASCI UART"
};

/*
 *	QUART (XR82C684) support
 */

#define MRA	0x00
#define SRA	0x01
#define CSRA	0x01
#define MISR1	0x02
#define CRA	0x02
#define RHRA	0x03
#define THRA	0x03
#define IPCR1	0x04
#define ACR1	0x04
#define ISR1	0x05
#define IMR1	0x05
#define CTU1	0x06
#define CTL1	0x07
#define MRB	0x08
#define SRB	0x09
#define CSRB	0x09
#define CRB	0x0A
#define RHRB	0x0B
#define THRB	0x0B
#define IVR1	0x0C
#define	IP1	0x0D
#define OPCR1	0x0D
#define SCC1	0x0E
#define SOPBC1	0x0E
#define STC1	0x0F
#define COPBC1	0x0F
#define MRC	0x10
#define SRC	0x11
#define CSRC	0x11
#define MISR2	0x12
#define CRC	0x12
#define RHRC	0x13
#define THRC	0x13
#define IPCR2	0x14
#define ACR2	0x14
#define ISR2	0x15
#define IMR2	0x15
#define CTU2	0x16
#define CTL2	0x17
#define MRD	0x18
#define SRD	0x19
#define CSRD	0x19
#define CRD	0x1A
#define RHRD	0x1B
#define THRD	0x1B
#define IVR2	0x1C
#define	IP2	0x1D
#define OPCR2	0x1D
#define SCC2	0x1E
#define SOPBC2	0x1E
#define STC2	0x1F
#define COPBC2	0x1F

#define	QUARTREG(x)	(((uint16_t)(x)) << 11)
#define QUARTPORT	0xBA

static int log = 0;

static uint8_t quart_intr(uint8_t minor)
{
	uint8_t d;
	/* FIXME: we need to check for OE status and if so issue CMD 0x40 */
	/* The QUART is really a pair of two port UARTs together */
	/*
	 *	ISR bits
	 *	7: Line state change - read IPCR1
	 *	6: Break delta (channel B)
	 *	5: Receiver ready/fifo full (depends on MR1B[6])
	 *	4: TX ready B (cleared by loading char)
	 *	3: Counter 1 ready
	 *	2: Break delta (channel A)
	 *	1: Receiver ready/fifo full (depends on MR1A[6])
	 *	0: TX ready A (cleared by loading char)
	 *
	 */
	uint8_t r = in16(QUARTPORT + QUARTREG(ISR1));
	if (r & 0x02) {
		d = in16(QUARTPORT + QUARTREG(RHRA));
		tty_inproc(minor, d);
	}
	if (r & 0x20) {
		d = in16(QUARTPORT + QUARTREG(RHRB));
                if (minor + 1 <= NUM_DEV_TTY)
			tty_inproc(minor + 1 , d);
	}
	/* ISR2 is the same for the other half - port C/D and counter 2 */
	r = in16(QUARTPORT + QUARTREG(ISR2));
	if (r & 0x02) {
		d = in16(QUARTPORT + QUARTREG(RHRC));
                if (minor + 2 <= NUM_DEV_TTY)
			tty_inproc(minor + 2 , d);
	}
	if (r & 0x20) {
		d = in16(QUARTPORT + QUARTREG(RHRD));
                if (minor + 3 <= NUM_DEV_TTY)
			tty_inproc(minor + 3, d);
	}
	if (timer_source == TIMER_QUART && (r & 0x08)) {
		/* Clear the timer interrupt - in timer mode it keeps
		   running so we don't need to reload the timer */
		in16(QUARTPORT + QUARTREG(STC2));
		/* Timer tick */
		do_timer_interrupt();
	}
	return 4;
}

/*

	We use the extend bit on the 82C684 to avoid all the nastiness
	with the older devices and baud rate combinations

   X   CSR
   0	0000		50
   1	0000		75
   0	0001		110
   0	0010		134.5
   1	0011		150
   0	0100		300
   0	0101		600
   0	0110		1200
   0	1000		2400
   0	1001		4800
   0	1011		9600
   1	1100		19200
   0	1100		38400
   1	0111		57600
   1	1000		115200

*/

#define XBIT		0x10

static const uint8_t baudmap[] = {
	0x07,		/* 0 is special */
	0x00,		/* 50 baud */
	XBIT|0x00,	/* 75 baud */
	0x01,		/* 110 baud */
	0x02,		/* 134 baud */
	XBIT|03,	/* 150 baud */
	0x04,		/* 300 baud */
	0x05,		/* 600 baud */
	0x06,		/* 1200 baud */
	0x08,		/* 2400 baud */
	0x09,		/* 4800 baud */
	0x0B,		/* 9600 baud */
	XBIT|0x0C,	/* 19200 baud */
	0x0C,		/* 38400 baud */
	XBIT|0x07,	/* 57600 baud */
	XBIT|0x08,	/* 115200 baud */
};

/*
 *	Program a QUART port the easy way
 */

static void quart_setup(uint8_t minor)
{
	struct termios *t = &ttydata[minor].termios;
	uint16_t port = ttyport[minor];
	uint8_t r = 0;
	uint8_t baud = t->c_cflag & CBAUD;

	if (!(t->c_cflag & PARENB))
		r = 0x10;	/* No parity */
	if (t->c_cflag & PARODD)
		r |= 0x04;
	/* 5-8 bits */
	r |= (t->c_cflag & CSIZE) >> 4;
	if (t->c_cflag & CRTSCTS)
		r |= 0x80;	/* RTS control on receive */

	/* Set pointer to MR1x */
	out16(port + QUARTREG(CRA), 0x10);
	out16(port + QUARTREG(MRA), r);

	if ((t->c_cflag & CSIZE)  == CS5)
		r  = 0x0;	/* One stop for 5 bit */
	else
		r = 0x07;	/* One stop bit */
	if (t->c_cflag & CRTSCTS)
		r |= 0x10;	/* CTS check on transmit */
	if (t->c_cflag & CSTOPB)
		r |= 0x08;	/* Two stop bits */


	log = 1;
	out16(port + QUARTREG(MRA), r);

	r = baudmap[baud];

	if (r & XBIT) {
		out16(port + QUARTREG(CRA), 0x80);
		out16(port + QUARTREG(CRA), 0xA0);
	} else {
		out16(port + QUARTREG(CRA), 0x90);
		out16(port + QUARTREG(CRA), 0xB0);
	}
	r |= (r << 4);
	out16(port + QUARTREG(CSRA), r);

out:
	/* Ensure TX and RX are enabled */
	out16(port + QUARTREG(CRA), 0x15);
	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;
}

static uint8_t quart_writeready(uint_fast8_t minor)
{
	uint16_t port = ttyport[minor];
	uint8_t r;
	r = in16(port + QUARTREG(SRA));
	if (r & 0x04)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

static void quart_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint16_t port = ttyport[minor];
	out16(port + QUARTREG(THRA), c);
}

/*
 *	QUART basic driver
 */

struct uart quart_uart = {
	quart_intr,
	quart_writeready,
	quart_putc,
	quart_setup,
	carrier_unwired,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|CRTSCTS|_CSYS,
	"QUART"
};

/*
 *	26C92 UART driver. We use this for a lot more than UART so don't
 *	touch anything we shouldn't as it may also be driving bit bang SPI SD
 *	cards.
 *
 *	To support the EXAR version we need a different initial set up and
 *	baud rate process.
 *
 *	The big differences are:
 *	- No MR0 on the 88C681
 *	- IM2 on 88C681
 *	- Some commands are different
 *	- It has useful per channel and per RX/TX BRG configuration bits
 *	- Less/different FIFO controls
 */

static uint8_t sc26c92_intr(uint8_t minor)
{
	uint8_t p = ttyport[minor];
	uint8_t r = in(p + ISR1);
	if (r & 0x02)
		tty_inproc(minor, in(p + RHRA));
	if (r & 0x20)
		tty_inproc(minor, in(p + RHRB));
	if (r & 0x10) {
		in(p + STC1);
		if (timer_source == TIMER_SC26C92)
			do_timer_interrupt();
	}
	return 2;
}

/* SC26C92 @7.372MHz */
static uint8_t sc26c92_baud[16] = {
	0x99,
	0xFF,	/* 50 */
	0xFF,	/* 75 */
	0xFF,	/* 110 */
	0xFF,	/* 134.5 */
	0x00,	/* 150 */
	0x33,	/* 300 */
	0x44,	/* 600 */
	0x55,	/* 1200 */
	0x66,	/* 2400 */
	0x88,	/* 4800 */
	0x99,	/* 9600 */
	0xBB,	/* 19200 */
	0xCC,	/* 38400 */
	0xFF,	/* 57600 */
	0xFF	/* 115200 */
};

/* XR88C681 @3.68MHz is like the quart so use that table */

/* For the SC26C92 this is a fairly simple approach just using the best table.
   We can in theory look for combinations to get high speed or if not using
   the timer use the timer to get one port on an otherwise unreachable rate.
   For the Exar parts it's easy we have per channel baud source select */

static void xrsc_setup(uint8_t minor, uint8_t exar)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t p = ttyport[minor];
	uint8_t b = p & 0xF0;	/* Base of card */
	uint8_t baud = t->c_cflag & CBAUD;
	uint8_t r;

	if (!(t->c_cflag & PARENB))
		r = 0x10;	/* No parity */
	if (t->c_cflag & PARODD)
		r |= 0x04;	/* Odd parity */
	r |= (t->c_cflag & CSIZE) >> 4;	/* Fill in the size bits */
	if (t->c_cflag & CRTSCTS)	/* Enable RTS control if CTSRTS in use */
		r |= 0x80;

	out(p + CRA, 0x10);	/* Select MR1 */
	out(p + MRA, r);	/* Set up MR1 */

	r = 0;
	if ((t->c_cflag & CSIZE) == CS5)
		r = 0;
	else
		r = 7;
	if (t->c_cflag & CRTSCTS)
		r = 0x10;	/* CTS enables transmitter */
	if (t->c_cflag & CSTOPB)
		r |= 0x08;	/* Two stop bits */
	out(p + MRA, r);	/* Set up MR2 */

	if (exar) {
		/* Assumes ACR[7] = 0 */
		r = baudmap[baud];
		if (r & XBIT) {
			out(p + CRA, 0x80);
			out(p + CRA, 0xA0);
		} else {
			out(p + CRA, 0x90);
			out(p + CRA, 0xB0);
		}
	} else {
		/* Baud handling. We for now just use baud 0, ACR 1 */
		r = sc26c92_baud[baud];
		/* Ones we cannot do yet */
		if (r == 0xFF) {
			t->c_cflag &= ~CBAUD;
			t->c_cflag |= B9600;
			r = 0x99;
		}
	}
	out(p + CSRA, r);	/* Speed */
	out(p + CRA, 0x05);	/* Enable RX and TX */

}

static void sc26c92_setup(uint8_t minor)
{
	xrsc_setup(minor, 0);
}

static void xr88c681_setup(uint_fast8_t minor)
{
	xrsc_setup(minor, 1);
}

static uint_fast8_t sc26c92_writeready(uint_fast8_t minor)
{
	uint8_t p = ttyport[minor];
	if (in(p + SRA) & 0x04)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

static void sc26c92_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t p = ttyport[minor];
	out(p + THRA, c);
}

/*
 *	SC26C92 and XR88C681 driver
 */

struct uart sc26c92_uart = {
	sc26c92_intr,
	sc26c92_writeready,
	sc26c92_putc,
	sc26c92_setup,
	carrier_unwired,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|CRTSCTS|_CSYS,
	"SC26C92"
};

struct uart xr88c681_uart = {
	sc26c92_intr,
	sc26c92_writeready,
	sc26c92_putc,
	xr88c681_setup,
	carrier_unwired,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|CRTSCTS|_CSYS,
	"XR88C681"
};

/*
 *	Console driver for the TMS9918A
 *	EF9345 is hooked into the TMS driver so this really
 *	needs renamign as vt driver..
 */

static uint8_t tms_intr(uint8_t minor)
{
	used(minor);
	return 1;
}

uint8_t outputtty = 1, inputtty = 1;
#define VT_CON	4
static struct vt_switch ttysave[VT_CON];

static void tms_setup(uint8_t minor)
{
	used(minor);
}

static uint8_t tms_writeready(uint_fast8_t minor)
{
	used(minor);
	return TTY_READY_NOW;
}

static void tms_setoutput(uint_fast8_t minor)
{
	vt_save(&ttysave[outputtty - 1]);
	outputtty = minor;
	/* The 9345 needs to cache the output bank */
	if (ef9345_present)
		ef9345_set_output();
	vt_load(&ttysave[outputtty - 1]);
}

static void tms_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

	if (outputtty != minor)
		tms_setoutput(minor);
	irqrestore(irq);
	if (!vswitch)
		vtoutput(&c, 1);
}

/* Callback from the keyboard driver for a console switch */
void do_conswitch(uint8_t c)
{
	/* No switch if the console is locked for graphics */
	if (vswitch)
		return;

	/* 80 column card */
	if (ef9345_present) {
		/* Calls into the EF9345 code. FIXME clean path */
		tms_setoutput(inputtty);
		vt_cursor_off();
		inputtty = c;
		set_console();
		vt_cursor_on();
		vt_twidth = 80;
		vt_tright = 79;
		return;
	}
	/* TMS9918A */
	tms_setoutput(inputtty);
	vt_cursor_off();
	inputtty = c;
	if (vidmode != mode[c])
		tms9918a_reload();
	else {
		tms9918a_udgload();
		tms9918a_attributes();
	}
	set_console();
	tms_setoutput(c);
	vt_cursor_on();
}

/* PS/2 call back for alt-Fx */
void ps2kbd_conswitch(uint8_t console)
{
	if (console > 4 || console == inputtty || vswitch)
		return;
	do_conswitch(console);
}

struct uart tms_uart = {
	tms_intr,
	tms_writeready,
	tms_putc,
	tms_setup,
	carrier_unwired,
	_CSYS,
	"TMS9918A"
};

/* The EF9345 is run via the same vt layer */

static void ef_setup(uint8_t minor)
{
	used(minor);
	vt_twidth = 80;
	vt_tright = 79;
}

struct uart ef_uart = {
	tms_intr,			/* TODO */
	tms_writeready,
	tms_putc,
	ef_setup,
	carrier_unwired,
	_CSYS,
	"EF9345"
};

/* FIXME: could move this routine into discard */

uint8_t register_uart(uint16_t port, struct uart *ops)
{
	queue_t *q = ttyinq + nuart;
	uint8_t *buf;
	if (nuart > NUM_DEV_TTY)
	    return 0;
	buf = code1_alloc(TTYSIZ);
	ttyport[nuart] = port;
	uart[nuart] = ops;
	q->q_base = q->q_head = q->q_tail = buf;
	q->q_size = TTYSIZ;
	q->q_count = 0;
	q->q_wakeup =  TTYSIZ/2;
	return nuart++;
}

/* Ditto into discard */

void insert_uart(uint16_t port, struct uart *ops)
{
	struct uart **p = &uart[NUM_DEV_TTY];
	uint16_t *pt = &ttyport[NUM_DEV_TTY];
	uint8_t *buf;

	/* Are we going to throw out a UART ? If so recycle the allocated
	   buffer, if not we need one */
	if (nuart > NUM_DEV_TTY) {
		buf = ttyinq[NUM_DEV_TTY].q_base;
		nuart--;
	} else
		buf = code1_alloc(TTYSIZ);

	while(p != uart + 1) {
		*p = p[-1];
		*pt = pt[-1];
		p--;
		pt--;
	}
	uart[1] = ops;
	ttyport[1] = port;
	first_poll++;
	ttyinq[1].q_base = ttyinq[1].q_head = ttyinq[1].q_tail = buf;
	ttyinq[1].q_size = TTYSIZ;
	ttyinq[1].q_count = 0;
	ttyinq[1].q_wakeup =  TTYSIZ/2;
	nuart++;
}

/* Ditto into discard */

void display_uarts(void)
{
	struct uart **t = &uart[1];
	uint16_t *p = ttyport + 1;
	uint8_t n = 1;

	while(n < nuart) {
		kprintf("tty%d: %s at 0x%x.\n", n, (*t)->name, *p);
		p++;
		t++;
		n++;
	}
}

/*
 *	Graphics layer interface (for TMS9918A and friends)
 */

static struct videomap tms_map = {
	0,
	0x98,
	0, 0,
	0, 0,
	1,
	MAP_PIO
};

static struct videomap ef_map = {
	0,
	0x42,
	0, 0,
	0, 0,
	2,
	MAP_PIO
};

static struct display ef_mode[1] = {
	{
		0,
		80, 24,
		80, 24,
		255, 255,
		FMT_TEXT,
		HW_EF9345,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,	/* TODO: vblank */
		16,
		0,
		80, 24
	}
};

/* FIXME: we need a way of reporting CPU speed/TMS delay info as unlike the
   ports so far we need delays on the RC2014 */

/*
 *	We always report a TMS9918A. Now it is possible that someone
 *	is using a 9938 or 9958 so we might want to add some VDP autodetect
 *	code and report accordingly but that's a minor TODO
 */
static struct display tms_mode[2] = {
	{
		0,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT|GFX_VBLANK,
		16,
		0,
		40, 24
	},
	{
		1,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT|GFX_VBLANK,
		16,
		0,
		32, 24
	}
};

/*
 *	TMS9918A configuration
 *	Text mode is wired into vdp1.s
 *
 *	0x3C00-0x3FFF are reserved by the OS.
 *
 *	The text mode configuration we use is
 *	Secret font store at 3C00-3FFF (128 base symbols copy)
 *	Secret UDG stash at 3800-3BFF in future maybe (32 chars x 4 so 1K)
 *	4 screens base 0x0000 + 0x400 per screen
 *	Patterns base 0x1000
 *
 *	For graphics:
 *		Sprite Patterns 0x1800
 *		Colour table 0x2000
 *		Sprite attribute 3B00-3BFF
 *	For graphics two non-standard we can maybe do similar ?
 */

static const uint8_t tmsreset[8] = {
	0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t tmstext[8] = {
	0x00, 0xD0, 0x00, 0x00, 0x02, 0x00, 0x00, 0xF4 /* Text, no IRQ */
};

static const uint8_t tmstext32[8] = {
	0x00, 0xC2, 0x00, 0x80, 0x02, 0x76, 0x03, 0x04/* Text, no IRQ */
};

/* Should move these helpers into asm TODO */
static void nap(void)
{
}

void tms9918a_config(const uint8_t *r)
{
	irqflags_t irq = di();
	uint8_t c = 0x80;
	while(c < 0x88) {
		tms9918a_ctrl = *r++;
		tms9918a_ctrl = c++;
		nap();
	}
	irqrestore(irq);
}

static uint8_t tms_readb(uint16_t addr)
{
	tms9918a_ctrl = addr;
	tms9918a_ctrl = addr >> 8;
	nap();
	return tms9918a_data;
}

static void tms_writeb(uint16_t addr, uint8_t data)
{
	tms9918a_ctrl = addr;
	tms9918a_ctrl = (addr >> 8) | 0x40;
	tms9918a_data = data;
}

/* Reset the TMS9918A and turn off its interrupt */
void tms9918a_reset(void)
{
	tms9918a_config(tmsreset);
}

void tms9918a_set_char(uint_fast8_t c, uint8_t *d)
{
	irqflags_t irq = di();
	unsigned addr = 0x1000 + 8 * c;
	uint_fast8_t i;
	for (i = 0; i < 8; i++)
		tms_writeb(addr++, *d++);
	irqrestore(irq);
}

void tms9918a_udgsave(void)
{
	irqflags_t irq = di();
	unsigned i;
	unsigned uaddr = 0x1400;	/* Char 128-159 */
	unsigned addr = 0x3800 + 256 * (inputtty - 1);
	for (i = 0; i < 256; i++)
		tms_writeb(addr, tms_readb(uaddr++));
	irqrestore(irq);
}

void tms9918a_udgload(void)
{
	irqflags_t irq = di();
	unsigned i;
	unsigned addr = 0x3800 + 256 * (inputtty - 1);
	unsigned uaddr = 0x1000;		/* Char 128-159, inverses at 0-31 for cursor */
	for (i = 0; i < 256; i++) {
		uint8_t c = tms_readb(uaddr++);
		tms_writeb(addr, ~c);
		tms_writeb(addr + 0x400, c);
	}
	irqrestore(irq);
}

/* Restore colour attributes */
void tms9918a_attributes(void)
{
	irqflags_t irq = di();
	if (mode[inputtty]) {
		unsigned addr;
		uint8_t c = tmsinkpaper[inputtty];
		addr = 0x2000;
		while(addr != 0x2020)
			tms_writeb(addr++, c);
		tms9918a_ctrl = tmsborder[inputtty];
		tms9918a_ctrl = 0x87;
	} else {
		tms9918a_ctrl = tmsinkpaper[inputtty];
		tms9918a_ctrl = 0x87;
	}
	irqrestore(irq);
}


void tms9918a_attributes_m(uint8_t minor)
{
	if (inputtty == minor)
		tms9918a_attributes();
}

struct tmsinfo {
	uint16_t lastline;
	uint16_t dmov;
	uint16_t s1;
	uint16_t w;
	uint16_t umov;
	uint8_t *conf;
	uint8_t inton;
};

static struct tmsinfo tmsdat[2] = {
	{
		0x03C0,
		0xFFD8,
		0x4028,
		0x0028,
		0x3FD8,
		tmstext,
		0xF0
	}, {
		0x02E0,
		0xFFE0,
		0x4020,
		0x0020,
		0x3FE0,
		tmstext32,
		0xE2
	}
};

/* This relies on the TMS9918A interrupt being off */
void tms9918a_reload(void)
{
	uint16_t r;
	uint8_t b;
	struct tmsinfo *t;

	vidmode = mode[inputtty];
	t = tmsdat + vidmode;

	for (r = 0; r < 0x400; r++) {
		b = tms_readb(0x3C00 + r);
		tms_writeb(0x1000 + r, b);
		tms_writeb(0x1400 + r , ~b);
	}
	tms_writeb(0x3B00, 0xD0);
	tms9918a_config(t->conf);
	vt_twidth = t->w;
	vt_tright = vt_twidth - 1;
	/* Set up the scrollers in the asm side */
	scrolld_base = t->lastline;	/* Start of last line */
	scrolld_mov = t->dmov;		/* How to move up */
	scrolld_s1 = t->s1;		/* Move back and turn on write */
	scrollu_w = t->w;		/* Start upscroll on line 1 */
	scrollu_mov = t->umov;		/* Move up 40 bytes, and add 0x4000 (write) */
	vdpport &= 0xFF;
	vdpport |= t->w  << 8;		/* Set up width counters */
	/* Turn on the IRQ if we need it */
	if (timer_source == TIMER_TMS9918A) {
		tms9918a_ctrl = t->inton;
		tms9918a_ctrl = 0x81;
	}
	tms9918a_udgload();
	tms9918a_attributes();
	vt_cursor_off();
	tms_setoutput(inputtty);
	vt_cursor_on();
	tms_mode[0].hardware = tms9918a_present;
	tms_mode[1].hardware = tms9918a_present;
}

static struct fontinfo fonti[] = {
	{ 0, 255, 128, 159, FONT_INFO_6X8 },
	{ 0, 255, 128, 159, FONT_INFO_8X8 },
};

static uint8_t igrbmsx[16] = {
	1,	/* 0000 to Black */
	4,	/* 000B to 4 dark blue */
	6,	/* 00R0 to 6 dark red */
	13,	/* 00RB to magneta */
	12,	/* 0G00 to dark green */
	7,	/* 0G0B to cyan */
	10,	/* 0GR0 to dark yellow */
	14,	/* 0GRB to grey */
	14,	/* I000 to grey */
	5,	/* I00B to light blue */
	9,	/* 10R0 to light red */
	8,	/* 10RB to magenta or medium red ? - use mr for now */
	3,	/* 1G00 to light green */
	7,	/* 1G0B to cyan - no light cyan */
	11,	/* 1GR0 to light yellow */
	15	/* 1GRB to white */
};

static uint8_t igrb_to_msx(uint8_t c)
{
	/* Machine specific colours */
	if (c & 0x10)
		return c & 0x0F;
	/* IGRB colours */
	return igrbmsx[c & 0x0F];
}

/* We can have up to 4 vt consoles or it may be shadowing serial input */
int rctty_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t map[8];
  unsigned i = 0;
  unsigned topchar = 256;
  uint8_t c;

  if (uart[minor] == &tms_uart) {
  	switch(arg) {
  	case GFXIOC_GETINFO:
                return uput(&tms_mode[mode[minor]], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		if (vswitch)
			return -EBUSY;
		vswitch = minor;
		return uput(&tms_map, ptr, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		if (vswitch == minor) {
			tms9918a_reset();
			tms9918a_reload();
			vswitch = 0;
		}
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE: {
		uint8_t m = ugetc(ptr);
		if (m > 1) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(&tms_mode[m], ptr, sizeof(struct display));
		mode[minor] = m;
		if (minor == inputtty)
			tms9918a_reload();
		return 0;
		}
	case GFXIOC_WAITVB:
		psleep(&shadowcon);
		return 0;
	case VDPIOC_SETUP:
		/* Must be locked to issue */
		if (vswitch != minor) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, map, 8) == -1)
			return -1;
		map[1] |= 0x80;
		if (timer_source == TIMER_TMS9918A)
			map[1] |= 0x20;
		tms9918a_config(map);
		return 0;
	case VDPIOC_READ:
	case VDPIOC_WRITE:
	{
		struct vdp_rw rw;
		uint16_t size;
		uint8_t *addr = (uint8_t *)rw.data;
			if (vswitch != minor) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, &rw, sizeof(struct vdp_rw)) != sizeof(struct vdp_rw)) {
			udata.u_error = EFAULT;
			return -1;
		}
		size = rw.lines * rw.cols;
		if (valaddr(addr, size) != size) {
			udata.u_error = EFAULT;
			return -1;
		}
		if (arg == VDPIOC_READ)
			udata.u_error = vdp_rop(&rw);
		else
			udata.u_error = vdp_wop(&rw);
		if (udata.u_error)
			return -1;
		return 0;
	}
	case VTFONTINFO:
		return uput(fonti + mode[minor], ptr, sizeof(struct fontinfo));
	case VTSETUDG:
		topchar = 128 + 32;
		i = 128;
	case VTSETFONT:
		while(i < topchar) {
			if (uget(ptr, map, 8) == -1) {
				udata.u_error = EFAULT;
				return -1;
			}
			ptr += 8;
			tms9918a_set_char(i++, map);
		}
		tms9918a_udgsave();
		tms9918a_udgload();
		return 0;
	case VTBORDER:
		c = ugetc(ptr);
		tmsborder[minor]  = igrb_to_msx(c & 0x1F);
		tms9918a_attributes_m(minor);
		return 0;
	case VTINK:
		c = ugetc(ptr);
		tmsinkpaper[minor] &= 0x0F;
		tmsinkpaper[minor] |= igrb_to_msx(c & 0x1F) << 4;
		tms9918a_attributes_m(minor);
		return 0;
	case VTPAPER:
		c = ugetc(ptr);
		tmsinkpaper[minor] &= 0xF0;
		tmsinkpaper[minor] |= igrb_to_msx(c & 0x1F);
		tms9918a_attributes_m(minor);
		return 0;
	case CPUIOC_Z80SOFT81:
		if (arg == 0)
			return softzx81_off(minor);
		else
			return softzx81_on(minor);
	}
  }
  if (uart[minor] == &ef_uart) {
	switch(arg) {
	case GFXIOC_MAP:
		return uput(&ef_map, ptr, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		return 0;
	case VTINK:
		c = ugetc(ptr);
		efinkpaper[minor] &= 0xF8;
		efinkpaper[minor] |= c & 0x07;
		if (minor == inputtty) {
			kprintf("setink %d\n", c);
			ef9345_colour(efinkpaper[minor]);
		}
		return 0;
	case VTPAPER:
		c = ugetc(ptr);
		efinkpaper[minor] &= 0x8F;
		efinkpaper[minor] |= (c & 0x07) << 4;
		if (minor == inputtty)
			ef9345_colour(efinkpaper[minor]);
		return 0;
	}
  }
  if (uart[minor] == &ef_uart || uart[minor] == &tms_uart) {
	  /* Only the ZX keyboard has support for the bitmapped matrix ops
	   and map setting. We need to add different map setting for PS/2
	   and different auto repeat if we support setting it */
	if (!zxkey_present &&
		( arg == KBMAPSIZE && arg == KBMAPGET || arg == KBSETTRANS ||
			arg == KBRATE ))
			return -1;
	return vt_ioctl(minor, arg, ptr);
  }
  /* Not a VT port so don't go via generic VT */
  return tty_ioctl(minor, arg, ptr);
}

int rctty_close(uint_fast8_t minor)
{
	if (vswitch == minor) {
		tms9918a_reset();
		tms9918a_reload();
		vswitch = 0;
	}
	return tty_close(minor);
}
