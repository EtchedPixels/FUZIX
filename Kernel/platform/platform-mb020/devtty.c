#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

void *ttyport[NUM_DEV_TTY + 1];
const struct uart *uart[NUM_DEV_TTY + 1];
uint8_t nuart = 1;

unsigned tty_asleep;

static uint8_t console_buf[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {
	{ },
	/* Must preallocate the console */
	{ console_buf, console_buf, console_buf, TTYSIZ, 0, TTYSIZ / 2 },
	/* Other ports dynamically allocated */
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS
};


void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	uart[minor]->setup(minor, flags);
}

int tty_carrier(uint_fast8_t minor)
{
	return uart[minor]->carrier(minor);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uart[minor]->putc(minor, c);
}

void tty_sleeping(uint_fast8_t minor)
{
	tty_asleep |= (1 << minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return uart[minor]->writeready(minor);
}

void tty_data_consumed(uint_fast8_t minor)
{
	uart[minor]->data_consumed(minor);
}

void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		kputchar('\r');
	while (tty_writeready(TTYDEV & 0xFF) != TTY_READY_NOW);
	tty_putc(TTYDEV & 0xFF, c);
}

int mtty_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor > NUM_DEV_TTY || ttyport[minor] == NULL) {
		udata.u_error = ENXIO;
		return -1;
	}
	return tty_open(minor, flag);
}

uint8_t register_uart(void *port, const struct uart *ops)
{
	queue_t *q = ttyinq + nuart;
	uint8_t *buf;
	if (nuart > NUM_DEV_TTY)
		return 0;

	ttyport[nuart] = port;
	uart[nuart] = ops;
	termios_mask[nuart] = ops->cmask;

	/* Check for preallocated buffers. We have to allocate the
	   console buffer in advance, because we need the console
	   before we have malloc up and running */

	if (q->q_base != NULL)
		return nuart++;

	/* Not preallocated - grab a buffer */
	buf = kmalloc(TTYSIZ, 0);
	if (buf == NULL) {
		kputs("out of memory for UART buffer.\n");
		return 0;
	}
	q->q_base = q->q_head = q->q_tail = buf;
	q->q_size = TTYSIZ;
	q->q_count = 0;
	q->q_wakeup = TTYSIZ / 2;
	return nuart++;
}

void display_uarts(void)
{
	const struct uart **t = &uart[1];
	void **p = ttyport + 1;
	uint8_t n = 1;

	while (n < nuart) {
		kprintf("tty%d: %s at 0x%p.\n", n, (*t)->name, *p);
		p++;
		t++;
		n++;
	}
}

/*
 *	UART implementations: could move to their own files ?
 */

/*
 *	National Semiconductor 16x50 series devices.
 *	TODO
 *	- Full RTS/CTS (needs a core change as the FIFO otherwise
 *	  makes us too late on the flow control assert)
 *	- Look at interrupt driven transmit and see if it is worth while
 */
struct uart16x50 {
	uint8_t data;		/* ls, rx, tx */
	uint8_t msier;		/* ms or ier */
	uint8_t fcr;		/* Also iir */
#define iir fcr
	uint8_t lcr;
	uint8_t mcr;
	uint8_t lsr;
	uint8_t msr;
	uint8_t scr;
};

#define U16X50(x) 	((volatile struct uart16x50 *)(ttyport[x]))

/* MSR */

#define CTS_D	0x01
#define DCD_D	0x08
#define CTS	0x10
#define DCD	0x80

/* LSR */
#define	BREAK	0x08
#define ERROR	0x71		/* OE PE FE or FIFO */

/* MCR */
#define DTR	0x01
#define RTS	0x02

/* IRR bits
 * 3 2 1 0
 * -------
 * x x x 1     no interrupt pending
 * 0 1 1 0  6  LSR changed -- read the LSR
 * 0 1 0 0  4  receive FIFO >= threshold
 * 1 1 0 0  C  received data sat in FIFO for a while
 * 0 0 1 0  2  transmit holding register empty
 * 0 0 0 0  0  MSR changed -- read the MSR
 */

static uint8_t ns16x50_intr(uint_fast8_t minor)
{
	volatile struct uart16x50 *uart = U16X50(minor);
	uint_fast8_t msr;
	uint_fast8_t iir;
	uint_fast8_t lsr;
	uint_fast8_t bad = 0;

	/* While there is an interrupt pending */
	while (((iir = uart->iir) & 1) == 0) {
		lsr = uart->lsr;
		switch (iir & 0x0E) {
		case 0x06:	/* LSR change */
			if (lsr & BREAK)
				tty_break_event(minor);
			if (lsr & ERROR)
				bad = 1;
			break;
		case 0x04:
		case 0x12:	/* Data */
			if (bad) {
				tty_inproc_bad(minor, uart->data);
				bad = 0;
			} else
				tty_inproc_full(minor, uart->data);
			break;
		case 0x02:	/* THR empty - don't care */
			break;
		case 0x00:	/* MSR change */
			msr = uart->msr;
			if (msr & DCD_D) {
				if (msr & DCD)
					tty_carrier_raise(minor);
				else
					tty_carrier_drop(minor);
			}
			/* Wake up any sleeping process */
			if ((msr & (CTS_D | CTS)) == (CTS_D | CTS))
				tty_outproc(minor);
			break;
		default:
		}
	}
	/* If we have room to receive then keep RTS on */
	/* TODO: this has too much latency on a FIFO */
	if (fullq(ttyinq + minor))
		uart->mcr &= ~RTS;
	return 1;
}

/* The core ate enough data that we can unthrottle the flow control */
static void ns16x50_data_consumed(uint_fast8_t minor)
{
	volatile struct uart16x50 *uart = U16X50(minor);
	uart->mcr = RTS;
}

static ttyready_t ns16x50_writeready(uint_fast8_t minor)
{
	volatile struct uart16x50 *uart = U16X50(minor);

	if (ttydata[minor].termios.c_cflag & CRTSCTS) {
		if ((uart->msr & CTS) == 0)
			return TTY_READY_LATER;
	}
	return uart->lsr & 0x20 ? TTY_READY_NOW : TTY_READY_SOON;
}

static void ns16x50_putc(uint_fast8_t minor, uint_fast8_t c)
{
	volatile struct uart16x50 *uart = U16X50(minor);
	uart->data = c;
}

/* 1.8432MHz */
static uint16_t clocks[] = {
	3,
	2304, /* B50 */
	1286, /* B75 */
	1047, /* B110 */
	859,  /* B134 */
	768,  /* B150 */
	384,  /* B300 */
	192,  /* B600 */
	96,   /* B1200 */
	48,   /* B2400 */
	24,   /* B4800 */
	12,   /* B9600 */
	6,    /* B19200 */
	3,    /* B38400 */
	2,    /* B57600 */
	1,    /* B115200 */
};

static void ns16x50_setup(uint_fast8_t minor, uint_fast8_t wait)
{
	volatile struct uart16x50 *uart = U16X50(minor);
	struct termios *t = &ttydata[minor].termios;
	uint8_t d;
	uint16_t w;

	d = 0x80;		/* DLAB (so we can write the speed) */
	d |= (t->c_cflag & CSIZE) >> 4;
	if (t->c_cflag & CSTOPB)
		d |= 0x04;	/* Two stop bits */
	if (t->c_cflag & PARENB)
		d |= 0x08;	/* Turn on parity */
	if (!(t->c_cflag & PARODD))
		d |= 0x10;	/* Odd parity */
	uart->lcr = d;		/* LCR */
	w = clocks[t->c_cflag & CBAUD];
	uart->data = w;		/* Actually the shadow clock registers */
	uart->msier = w >> 8;
	/* Switch to the data map */
	uart->lcr = d & 0x7F;
	/* TODO proper FIFO control */
	/* TODO wait for FIFO drain if wait set */
	if (w >> 8)		/* Low speeds interrupt every byte for latency */
		uart->fcr = 0x00;
	else			/* High speeds set our interrupt quite early
				   as our latency is poor, turn on 64 byte if
				   we have a 16C750 */
		uart->fcr = 0x51;
	uart->mcr = DTR | RTS;
	/*
	 *      On a 12MHz 68008 we might want to do real TX ints. Needs
	 *      further consideration.
	 */
	uart->msier = 0x0D;	/* No TX interrupts */
}

static uint8_t ns16x50_carrier(uint_fast8_t minor)
{
	volatile struct uart16x50 *uart = U16X50(minor);
	return (uart->msr & DCD) ? 1 : 0;
}

const struct uart ns16x50_uart = {
	ns16x50_intr,
	ns16x50_writeready,
	ns16x50_putc,
	ns16x50_setup,
	ns16x50_carrier,
	ns16x50_data_consumed,
	/* TOOD: CRTSCTS : can we do mark/space ? */
	CSIZE | CBAUD | CSTOPB | PARENB | PARODD | _CSYS,
	"16x50"
};

/*
 *	RC2014 interrupt - scan the tty devices
 */
void rc2014_interrupt(void)
{
	unsigned n = 0;
	while(++n < nuart)
		uart[n]->intr(n);
}
