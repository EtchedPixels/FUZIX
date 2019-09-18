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
static uint8_t nuart = 1;
uint8_t ttyport[NUM_DEV_TTY + 1];

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
	uart[minor]->setup(minor, ttyport[minor]);
}

int tty_carrier(uint_fast8_t minor)
{
	return uart[minor]->carrier(minor, ttyport[minor]);
}

/* Poll each present UART. The helper returns a value to add which means
   we can do
   0 : call me again
   1 : normal
   2 : skip next port too (used with SIO)

   The rx side wants moving to proper interrupt driven queues */

void tty_pollirq(void)
{
	uint8_t minor = 1;
	while(minor < nuart)
		minor += uart[minor]->intr(minor, ttyport[minor]);
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
        uart[minor]->putc(minor, ttyport[minor], c);
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return uart[minor]->writeready(minor, ttyport[minor]);
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

static uint8_t acia_intr(uint8_t minor, uint8_t port)
{
	uint8_t ca = ACIA_C;
	used(port);
	if (ca & 1)
		tty_inproc(minor, ACIA_D);
	return 1;
}

static void acia_setup(uint8_t minor, uint8_t port)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r = t->c_cflag & CSIZE;

	used(port);
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

static uint8_t acia_writeready(uint_fast8_t minor, uint_fast8_t p)
{
	used(minor);
	used(p);

	if (ACIA_C & 0x02)	/* THRE? */
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

static void acia_putc(uint_fast8_t minor, uint_fast8_t p, uint_fast8_t c)
{
	used(minor);
	used(p);

	ACIA_D = c;
}

static uint8_t carrier_unwired(uint_fast8_t minor, uint_fast8_t p)
{
	used(minor);
	used(p);
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

static uint8_t sio_intrb(uint_fast8_t minor, uint_fast8_t p)
{
	uint8_t r;
	r = in(p);
	if ((r & 1) && !fullq(&ttyinq[minor]))
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

static uint8_t sio_intr(uint_fast8_t minor, uint_fast8_t p)
{
	uint8_t r;
	r = in(p);
	if (!(r & 2))
		return 2;
	return sio_intrb(minor, p);
}

/* Be careful here. We need to peek at RR but we must be sure nobody else
   interrupts as we do this. Really we want to switch to irq driven tx ints
   on this platform I think. Need to time it and see

   An asm common level tty driver might be a better idea

   Need to review this we should be ok as the IRQ handler always leaves
   us pointing at RR0 */
static ttyready_t sio_writeready(uint_fast8_t minor, uint_fast8_t p)
{
	irqflags_t irq;
	uint8_t c;

	used(minor);

	irq = di();
	out(p, 0);
	c = in(p);
	irqrestore(irq);

	if (c & 0x04)	/* THRE? */
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}


static void sio_putc(uint_fast8_t minor, uint_fast8_t p, uint_fast8_t c)
{
	used(minor);
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

static void sio_setup(uint_fast8_t minor, uint_fast8_t p)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;

	used(minor);

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

static uint8_t sio_carrier(uint_fast8_t minor, uint_fast8_t p)
{
        uint8_t c;

        used(minor);

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

static uint8_t ns16x50_intr(uint_fast8_t minor, uint_fast8_t port)
{
	uint8_t msr;
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

static ttyready_t ns16x50_writeready(uint_fast8_t minor, uint_fast8_t port)
{
	uint8_t n = in(port + 5);
	used(minor);
	return n & 0x20 ? TTY_READY_NOW : TTY_READY_SOON;
}

static void ns16x50_putc(uint_fast8_t minor, uint_fast8_t port, uint_fast8_t c)
{
	used(minor);
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


static void ns16x50_setup(uint_fast8_t minor, uint_fast8_t port)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;

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

static uint8_t ns16x50_carrier(uint_fast8_t minor, uint_fast8_t port)
{
	used(minor);
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

static uint8_t asci_intr0(uint_fast8_t minor, uint_fast8_t port)
{
	used(port);
	while(ASCI_STAT0 & 0x80)
		tty_inproc(minor, ASCI_RDR0);
	return 1;
}

static uint8_t asci_intr1(uint_fast8_t minor, uint_fast8_t port)
{
	used(port);
	while(ASCI_STAT1 & 0x80)
		tty_inproc(minor, ASCI_RDR1);
	return 1;
}

ttyready_t asci_writeready0(uint_fast8_t minor, uint_fast8_t port)
{
	used(minor);
	used(port);
        if (ASCI_STAT0 & 0x02)
	        return TTY_READY_NOW;
	return TTY_READY_SOON;
}

ttyready_t asci_writeready1(uint_fast8_t minor, uint_fast8_t port)
{
	used(minor);
	used(port);
        if (ASCI_STAT1 & 0x02)
	        return TTY_READY_NOW;
	return TTY_READY_SOON;
}


static void asci_putc0(uint_fast8_t minor, uint_fast8_t port, uint_fast8_t c)
{
	used(minor);
	used(port);
	ASCI_TDR0 = c;
}

static void asci_putc1(uint_fast8_t minor, uint_fast8_t port, uint_fast8_t c)
{
	used(minor);
	used(port);
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

static void asci_setup(uint_fast8_t minor, uint_fast8_t port)
{
    struct termios *t = &ttydata[minor].termios;
    uint8_t cntla = 0x60;
    uint8_t cntlb = 0;
    uint16_t cflag = t->c_cflag;
    uint8_t baud;
    uint8_t ecr = 0;

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

    if (minor == 1) {
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
    } else if (minor == 2) {
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

static uint8_t tms_intr(uint8_t minor, uint8_t port)
{
	used(minor);
	used(port);
	return 1;
}

/*
 *	Console driver for the TMS9918A
 */
static void tms_setup(uint8_t minor, uint8_t port)
{
	used(minor);
	used(port);
}

static uint8_t tms_writeready(uint_fast8_t minor, uint_fast8_t p)
{
	used(minor);
	used(p);
	return TTY_READY_NOW;
}

static void tms_putc(uint_fast8_t minor, uint_fast8_t p, uint_fast8_t c)
{
	used(minor);
	used(p);
	vtoutput(&c, 1);
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

uint8_t register_uart(uint8_t port, struct uart *ops)
{
	queue_t *q = ttyinq + nuart;
	uint8_t *p = code1_alloc(TTYSIZ);
	if (p == NULL || nuart > NUM_DEV_TTY)
		return 0;
	ttyport[nuart] = port;
	uart[nuart] = ops;
	q->q_base = q->q_head = q->q_tail = p;
	q->q_size = TTYSIZ;
	q->q_count = 0;
	q->q_wakeup =  TTYSIZ/2;
	return nuart++;
}

/* Ditto into discard */

void insert_uart(uint8_t port, struct uart *ops)
{
	struct uart **p = &uart[NUM_DEV_TTY];
	while(p != uart + 1) {
		*p = p[-1];
		p--;
	}
	uart[1] = ops;
	ttyport[1] = port;
	ttyinq[1].q_base = ttyinq[1].q_head = ttyinq[1].q_tail = init_alloc(TTYSIZ);
	ttyinq[1].q_size = TTYSIZ;
	ttyinq[1].q_count = 0;
	ttyinq[1].q_wakeup =  TTYSIZ/2;
	/* We may have booted one out */
	if (nuart < NUM_DEV_TTY)
		nuart++;
}

/* Ditto into discard */

void display_uarts(void)
{
	struct uart **t = &uart[1];
	uint8_t *p = ttyport + 1;
	uint8_t n = 1;

	while(n++ < nuart) {
		kprintf("%s detected at 0x%2x.\n", (*t)->name, *p);
		p++;
		t++;
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
  }
  return vt_ioctl(minor, arg, ptr);
}
