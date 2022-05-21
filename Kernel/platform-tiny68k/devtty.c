#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];

#define UART_MRA	0x01
#define UART_SRA	0x03
#define UART_CSRA	0x03
#define UART_CRA	0x05
#define UART_RHRA	0x07
#define UART_THRA	0x07
#define UART_IPCR	0x09
#define UART_ACR	0x09
#define UART_ISR	0x0B
#define UART_IMR	0x0B
#define UART_CTU	0x0D
#define UART_CTUR	0x0D
#define UART_CTL	0x0F
#define UART_CTLR	0x0F
#define UART_MRB	0x11
#define UART_SRB	0x13
#define UART_CSRB	0x13
#define UART_CRB	0x15
#define UART_RHRB	0x17
#define UART_THRB	0x17
#define UART_IVR	0x19
#define UART_OPCR	0x1B
#define UART_STARTCTR	0x1D
#define UART_SETOPR	0x1D
#define UART_STOPCTR	0x1F
#define UART_CLROPR	0x1F

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

static uint8_t sleeping;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS|CBAUD|PARENB|PARODD|CSTOPB|CSIZE|CRTSCTS,
	_CSYS|CBAUD|PARENB|PARODD|CSTOPB|CSIZE|CRTSCTS,
};

static volatile uint8_t *uart_base = (volatile uint8_t *)0xFFF000;

#define GETB(x)		(uart_base[(x)])
#define PUTB(x,y)	uart_base[(x)] = (y)

/* Output for the system console (kprintf etc). Polled. */
void kputchar(uint8_t c)
{
	if (c == '\n') {
		while(!(GETB(UART_SRA) &  4));
		PUTB(UART_THRA, '\r');
	}
	while(!(GETB(UART_SRA) &  4));
	PUTB(UART_THRA, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c = GETB(0x10 * (minor - 1) + UART_SRA);
	return (c & 4) ? TTY_READY_NOW : TTY_READY_SOON; /* TX DATA empty */
}

void tty_putc(uint8_t minor, unsigned char c)
{
	PUTB(0x10 * (minor - 1) + UART_THRA, c);
}

/* We have four BRG set ups but they affect both ports together so this
   is tricky stuff to do right as we need to check if we can do the wanted
   rates on both ports if both open.

   The and of baudsrc of eac valid port tells us which table and BRG settings
   to use. We must track the BRG as we can only toggle it not read it */

/* Which baud generators can generate each rate ?:
	bit 0		Normal BRG ACR:7 0
	bit 1		Normal BRG ACR:7 1
	bit 2		BRG Test ACR:7 0
			(No useful extra rates on ACR:7 1 with BRG test

   If we can't achieve a rate without breaking another open port we keep the
   old rate */

static const uint8_t baudsrc[] = {
	0x07,		/* 0 is special */
	0x01,		/* 50 baud is BRG1 ACR 0 */
	0x03,		/* 70 baud is BRG1 ACR 1 */
	0x03,		/* 110 is BRG1 ACR 0 or ACR 1 */
	0x03,		/* 134 likewise */
	0x02,		/* 150 BRG1 ACR 1 only */
	0x03,		/* 300 BRG1 ACR 0 or ACR 1 */
	0x03,		/* 600 ditto */
	0x03,		/* 1200 ditto */
	0x03,		/* 2400 ditto */
	0x07,		/* 4800 is available on all */
	0x07,		/* 9600 is available on all */
	0x06,		/* 19200 is available on all but BRG1 ACR 0 */
	0x05,		/* 38400 requires ACR 0 */
	0x04,		/* 57600 requires BRG 2 */
	0x04,		/* 11520 requires BRG 2 */
};
/* BRG values per speed for each table */
static const uint8_t baudset[3][16] = {
	{
		/* Normal BRG ACR:7 = 0 */
		0xDD,		/* 0 is special */
		0x00,		/* 50 */
		0x00,		/* Can't do 75 on the standard BRG */
		0x11,		/* 110 */
		0x22,		/* 134 */
		0x22,		/* No 150 on the standard BRG */
		0x44,		/* 300 */
		0x55,		/* 600 */
		0x66,		/* 1200 */
		0x88,		/* 2400 */
		0x99,		/* 4800 */
		0xBB,		/* 9600 */
		0xBB,		/* Can't do 19200 */
		0xCC,		/* 38400 */
		0xCC,		/* No 57600 */
		0xCC,		/* No 115200 */
	},{
		/* Normal BRG ACR:7 = 1 */
		0xDD,		/* 0 is special */
		0x00,		/* No 50 */
		0x00,		/* 75 */
		0x11,		/* 110 */
		0x22,		/* 134 */
		0x33,		/* 150 */
		0x44,		/* 300 */
		0x55,		/* 600 */
		0x66,		/* 1200 */
		0x88,		/* 2400 */
		0x99,		/* 4800 */
		0xBB,		/* 9600 */
		0xCC,		/* 19200  */
		0xCC,		/* No 38400 */
		0xCC,		/* No 57600 */
		0xCC,		/* No 115200 */
	},{
		/* BRG Test ACR:7 = 0 */
		0xDD,		/* 0 is special */
		0x00,		/* No 50 */
		0x00,		/* No 75 */
		0x00,		/* No 110 */
		0x00,		/* No 134 */
		0x00,		/* No 150 */
		0x00,		/* No 300 */
		0x00,		/* No 600 */
		0x00,		/* No 1200 */
		0x00,		/* No 2400 */
		0x00,		/* 4800 */
		0xBB,		/* 9600 */
		0x33,		/* 19200 */
		0xCC,		/* 38400 */
		0x55,		/* 57600 */
		0x66,		/* 115200 */
	}
};

/* FIXME: if we got to IRQ based transmit we need to lock against this
   FIXME: need to drain both tx FIFOs if switching BRG ?
 */

void tty_setup(uint8_t minor, uint8_t flags)
{
	uint8_t other = 2 - minor;

	struct termios *t = &ttydata[minor].termios;
	struct tty *to = &ttydata[other];

	uint8_t baud = t->c_cflag & CBAUD;
	uint8_t r = 0;
	uint8_t base = (minor - 1) * 0x10;
	uint8_t table = baudsrc[baud];

	static uint8_t oldbaud[3] = {0., B38400, B38400 };
	static uint8_t oldbrg;

	/* If both ports are open we need to check the baud rate pair is
	   achievable */
	if (to->users)
		table &= baudsrc[to->termios.c_cflag & CBAUD];

	/* Clashing rates - use the old rate for this port */
	if (table == 0) {
		baud = oldbaud[minor];
		table = baudsrc[baud];
		goto out;
	}

	/* Favour the standard BRG set up, don't mix and match */
	if (table != 4) {
		if (table & 2) {
			PUTB(UART_ACR, 0xF0);
			table = 2;
		} else {
			PUTB(UART_ACR, 0x70);
			table = 1;
		}
	}


	if ((table & 4) != oldbrg) {
		GETB(UART_CRA);	/* Toggle BRG to use */
		oldbrg = table & 4;
	}

	/* Turn the mask into an actual table */
	if (table & 4)
		table = 2;
	else
		table = (table & 2) ? 1 : 0;

	if (!(t->c_cflag & PARENB))
		r = 0x08;	/* No parity */
	if (t->c_cflag & PARODD)
		r |= 0x04;
	/* 5-8 bits */
	r |= (t->c_cflag & CSIZE) >> 4;
	if (t->c_cflag & CRTSCTS)
		r |= 0x80;	/* RTS control on receive */

	/* Set pointer to MR1x */
	PUTB(base + UART_CRA, 0x10);
	PUTB(base + UART_MRA, r);

	if ((t->c_cflag & CSIZE)  == CS5)
		r  = 0x0;	/* One stop for 5 bit */
	else
		r = 0x07;	/* One stop bit */
	if (t->c_cflag & CRTSCTS)
		r |= 0x10;	/* CTS check on transmit */
	if (t->c_cflag & CSTOPB)
		r |= 0x08;	/* Two stop bits */
	PUTB(base + UART_MRA, r);

	/* Work out the CSR for the baud: ACR is already set to 0x70 */
	oldbaud[minor] = baud;
	PUTB(base + UART_CSRA, baudset[table][baud]);
out:
	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;
}

/* Not wired on this board */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= 1 << minor;
}

void tty_data_consumed(uint8_t minor)
{
}

static void tty_interrupt(uint8_t r)
{
	if (r & 0x02) {
		r = GETB(UART_RHRA);
		tty_inproc(1,r);
	}	
	if (r & 0x01) {
		if (sleeping & 2)
			tty_outproc(1);
	}
	if (r & 0x04)
		PUTB(UART_CRA, 0x50);
	if (r & 0x20) {
		r = GETB(UART_RHRB);
		tty_inproc(2,r);
	}	
	if (r & 0x10) {
		if (sleeping & 4)
			tty_outproc(2);
	}
	if (r & 0x40)
		PUTB(UART_CRB, 0x50);
}

void plt_interrupt(void)
{
	uint8_t r = GETB(UART_ISR);
	static uint8_t c = 0x12;
	static uint8_t ct;
	tty_interrupt(r);
	if (r & 0x08) {
		/* Ack the interrupt */
		GETB(UART_STOPCTR);
		timer_interrupt();
		if (++ct == 20) {
			ct = 0;
			PUTB(UART_SETOPR, 0xFF);
			c <<= 1;
			c &= 0x7F;
			if (c == 0x10)
				c |= 0x02;
			PUTB(UART_CLROPR, c | (udata.u_insys ? 0x80 : 0));
		}
	}
	/* and 0x80 is the GPIO */
}
