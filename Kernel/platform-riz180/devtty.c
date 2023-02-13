#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <z180.h>

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS | CBAUD | PARENB | PARODD | CSIZE | CSTOPB | CRTSCTS,
	_CSYS | CBAUD | PARENB | PARODD | CSIZE | CSTOPB | CRTSCTS,
};

/* bit 5: turn on divide by 30 v 10
   bit 3: turn on scale by 64 not 16
   bit 2-0: 2^n for scaling (not 111) */
static const uint8_t baudtable[] = {
	/* Dividers for our clock. Table is smaller than the maths by far */
	0,
#if CONFIG_CPU_CLK == 18432000
	0xFF,			/* 50 */
	0xFF,			/* 75 */
	0xFF,			/* 110 */
	0xFF,			/* 134.5 */
	/* Divide by 30 and 64 */
	0x2E,			/* 150 */
	0x2D,			/* 300 */
	0x2C,			/* 600 */
	0x2B,			/* 1200 */
	0x2A,			/* 2400 */
	0x29,			/* 4800 */
	0x28,			/* 9600 */
	/* Divide by 30 and 16 */
	0x21,			/* 19200 */
	0x20,			/* 38400 */
	/* Divide by 10 and 16 */
	0x01,			/* 57600 */
	0x00,			/* 115200 */
#elif CONFIG_CPU_CLK == 9216000
	0xFF,			/* 50 */
	/* Divide by 30 and 64 */
	0x2E,			/* 75 */
	0xFF,			/* 110 */
	0xFF,			/* 134.5 */
	0x2D,			/* 150 */
	0x2C,			/* 300 */
	0x2B,			/* 600 */
	0x2A,			/* 1200 */
	0x29,			/* 2400 */
	0x28,			/* 4800 */
	/* Divide by 30 and 16 */
	0x21,			/* 9600 */
	0x20,			/* 19200 */
	0xFF,			/* 38400 */
	/* And 10x scaler */
	0x00,			/* 57600 */
	0xFF,			/* 115200 */
#elif CONFIG_CPU_CLK == 6144000
	0x2E,			/* 50 */
	0xFF,			/* 75 */
	0xFF,			/* 110 */
	0xFF,			/* 134.5 */
	/* Divide by 10 and 64 */
	0x0E,			/* 150 */
	0x0D,			/* 300 */
	0x0C,			/* 600 */
	0x0B,			/* 1200 */
	0x0A,			/* 2400 */
	0x09,			/* 4800 */
	0x08,			/* 9600 */
	/* Divide by 10 and 16 */
	0x01,			/* 19200 */
	0x00,			/* 38400 */
	0xFF,			/* 57600 */
	0xFF,			/* 115200 */
#else
#error "Need baud table"
#endif
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t cntla = 0x60;
	uint8_t cntlb;
	uint16_t cflag = t->c_cflag;
	uint8_t baud;
	uint8_t ecr = 0;

	used(flags);

	/* Handle the baud table. Right now this is hardcoded for our clock */
	baud = cflag & CBAUD;
	cntlb = baudtable[baud];

	/* Unachieveable rates we turn to 9600 as we can do that
	   on all our clocks */
	if (cntlb == 0xFF) {
		baud = B9600;
		cntlb = baudtable[B9600];
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= B9600;
	}

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


	if (minor == 1) {
		if (cflag & CRTSCTS)
			ecr = 0x20;
		/* FIXME: need to do software RTS side */
	} else {
		cflag &= ~CRTSCTS;
	}

	t->c_cflag = cflag;

	/* ASCI serial set up */
	if (minor == 1) {
		ASCI_CNTLA0 = cntla;
		ASCI_CNTLB0 = cntlb;
		ASCI_ASEXT0 &= ~0x20;
		ASCI_ASEXT1 |= ecr;
	} else if (minor == 2) {
		ASCI_CNTLA1 = cntla;
		ASCI_CNTLB1 = cntlb;
	}
}

/* Not unfortunately wired */
int tty_carrier(uint_fast8_t minor)
{
	minor;
	return 1;
}

void tty_pollirq_asci0(void)
{
	while (ASCI_STAT0 & 0x80)
		tty_inproc(1, ASCI_RDR0);
	if (ASCI_STAT0 & 0x70)
		ASCI_CNTLA0 &= ~0x08;
}

void tty_pollirq_asci1(void)
{
	while (ASCI_STAT1 & 0x80)
		tty_inproc(2, ASCI_RDR1);
	if (ASCI_STAT1 & 0x70)
		ASCI_CNTLA1 &= ~0x08;
}

/* FIXME: we should have a proper tty buffer output queue really */
void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	switch (minor) {
	case 1:
		ASCI_TDR0 = c;
		break;
	case 2:
		ASCI_TDR1 = c;
		break;
	}
}

void tty_sleeping(uint_fast8_t minor)
{
	minor;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t r;
	switch (minor) {
	case 1:
		r = ASCI_STAT0;
		break;
	case 2:
		r = ASCI_STAT1;
		break;
	}
	if (r & 0x02)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
	while (tty_writeready(TTYDEV & 0xFF) != TTY_READY_NOW);
	tty_putc(TTYDEV & 0xFF, c);
	if (c == '\n') {
		while (tty_writeready(TTYDEV & 0xFF) != TTY_READY_NOW);
		tty_putc(TTYDEV & 0xFF, '\r');
	}
}
