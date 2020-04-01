#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <graphics.h>
#include <rc2014.h>
#include <vt.h>
#include "z180_uart.h"
#include "vfd-term.h"
#include "vfd-debug.h"

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
};

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
#ifdef CONFIG_VFD_TERM
	if (minor == 1)
		vfd_term_write(c);
#endif
        /* If we have a video display then the first console output will
	   go to it as well *unless it has a keyboard too* */
        if (minor == 1 && shadowcon)
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
	if (ttyport[minor])
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

static uint16_t siobaud[] = {
	0xC0,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0,	/* 134 */
	0,	/* 150 */
	0xC0,	/* 300 */
	0x60,	/* 600 */
	0xC0,	/* 1200 */
	0x60,	/* 2400 */
	0x30,	/* 4800 */
	0x18,	/* 9600 */
	0x0C,	/* 19200 */
	0x06,	/* 38400 */
	0x04,	/* 57600 */
	0x02	/* 115200 */
};

static void sio_setup(uint_fast8_t minor)
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
	if (ctc_present && p == SIOB_C) {
		CTC_CH1 = 0x55;
		CTC_CH1 = siobaud[baud];
		if (baud > B600)	/* Use x16 clock and CTC divider */
			r = 0x44;
	} else
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
	ASCI_TDR0 = c;
}

static void asci_putc1(uint_fast8_t minor, uint_fast8_t c)
{
	used(minor);
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
            cntlb |= 4;
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
		r = in16(QUARTPORT + QUARTREG(RHRA));
		tty_inproc(minor, r);
	}
	if (r & 0x20) {
		r = in16(QUARTPORT + QUARTREG(RHRB));
                if (minor + 1 <= NUM_DEV_TTY)
			tty_inproc(minor + 1 , r);
	}
	/* ISR2 is the same for the other half - port C/D and counter 2 */
	r = in16(QUARTPORT + QUARTREG(ISR2));
	if (r & 0x02) {
		r = in16(QUARTPORT + QUARTREG(RHRC));
                if (minor + 2 <= NUM_DEV_TTY)
			tty_inproc(minor + 2 , r);
	}
	if (r & 0x20) {
		r = in16(QUARTPORT + QUARTREG(RHRD));
                if (minor + 3 <= NUM_DEV_TTY)
			tty_inproc(minor + 3, r);
	}
	if (quart_timer && (r & 0x10)) {
		/* Clear the timer interrupt - in timer mode it keeps
		   running so we don't need to reload the timer */
		in16(QUARTPORT + QUARTREG(STC2));
		/* Timer tick */
		timer_interrupt();
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
	uint8_t r;
	uint8_t baud = t->c_cflag & CBAUD;

	if (!(t->c_cflag & PARENB))
		r = 0x08;	/* No parity */
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
 *	Console driver for the TMS9918A
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
	vt_load(&ttysave[outputtty - 1]);
}

static void tms_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

	if (outputtty != minor)
		tms_setoutput(minor);
	irqrestore(irq);
	vtoutput(&c, 1);
}

/* Callback from the keyboard driver for a console switch */
void do_conswitch(uint8_t c)
{
	tms_setoutput(inputtty);
	vt_cursor_off();
	inputtty = c;
	set_console();
	tms_setoutput(c);
	vt_cursor_on();
}

/* PS/2 call back for alt-Fx */
void ps2kbd_conswitch(uint8_t console)
{
	if (console > 4 || console == inputtty)
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

/* FIXME: could move this routine into discard */

uint8_t register_uart(uint16_t port, struct uart *ops)
{
	queue_t *q = ttyinq + nuart;
	uint8_t *buf = code1_alloc(TTYSIZ);
	if (buf == NULL || nuart > NUM_DEV_TTY)
		return 0;
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
	uint8_t *buf = code1_alloc(TTYSIZ);
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
	/* We may have booted one out */
	if (nuart <= NUM_DEV_TTY)
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
 *	Graphics layer interface (for TMS9918A)
 */

static const struct videomap tms_map = {
	0,
	0x98,
	0, 0,
	0, 0,
	1,
	MAP_PIO
};

/* FIXME: we need a way of reporting CPU speed/TMS delay info as unlike the
   ports so far we need delays on the RC2014 */
static const struct display tms_mode = {
    1,
    256, 192,
    256, 192,
    255, 255,
    FMT_VDP,
    HW_VDP_9918A,
    GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
    16,
    0
};

int rctty_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  if (minor == 1 && tms9918a_present) {
  	switch(arg) {
  	case GFXIOC_GETINFO:
  		return uput(&tms_mode, ptr, sizeof(struct display));
	case GFXIOC_MAP:
		return uput(&tms_map, ptr, sizeof(struct display));
	case GFXIOC_UNMAP:
		return 0;
	}
	/* Only the ZX keyboard has support for the bitmapped matrix ops
	   and map setting.  We need to add different map setting for PS/2
	   and different auto repeat if we support setting it */
	if (!zxkey_present &&
		( arg == KBMAPSIZE && arg == KBMAPGET || arg == KBSETTRANS ||
			arg == KBRATE ))
			return -1;
  }
  return vt_ioctl(minor, arg, ptr);
}
