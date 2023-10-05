/*
 *	We have a 16x50 UART at 0x68 and maybe a PropIO2 at A8
 *
 *	TODO:
 *	- Hardware flow control
 *	- Support for abuse of 16x50 as interrupt controller
 *	- Support for timer hack
 *
 *	This file implements the serial ports for the platform. Fuzix implements
 *	a reasonable subset of the System 5 termios. Certain things that are
 *	rarely relevant like XCASE, delay fills and parity are left to the
 *	driver if desired.
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <propio2.h>

/* The 16x50 UART I/O ports */
__sfr __at 0x68 uart_tx;
__sfr __at 0x68 uart_rx;
__sfr __at 0x68 uart_ls;
__sfr __at 0x69 uart_ier;
__sfr __at 0x69 uart_ms;
__sfr __at 0x6A uart_fcr;
__sfr __at 0x6B uart_lcr;
__sfr __at 0x6C uart_mcr;
__sfr __at 0x6D uart_lsr;
__sfr __at 0x6E uart_msr;
__sfr __at 0x6F uart_scr;

/*
 *	One buffer for each tty
 */
static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];

static uint8_t sleeping;

/*
 *	TTY masks - define which bits can be changed for each port
 */

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	_CSYS
};


/*
 *	One entry per tty. The 0th entry is never used as tty minor 0 is
 *	special (/dev/tty) and it's cheaper to waste a few bytes that keep
 *	doing subtractions.
 */
struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};


/* Updated early in boot to 0,2,1 if PropIO present. This table works both
   ways purely because of the possible mappings. If that changes we'll need
   a forward and backward table. Most platforms have a fixed idea of the console
   so don't need this remapping layer */
uint8_t ttymap[NUM_DEV_TTY + 1] = {
	0, 1, 2
};

/* Write to system console. This is the backend to all the kernel messages,
   kprintf(), panic() etc. */

void kputchar(uint_fast8_t c)
{
	while(tty_writeready(1) != TTY_READY_NOW);
	if (c == '\n')
		tty_putc(1, '\r');
	while(tty_writeready(1) != TTY_READY_NOW);
	tty_putc(1, c);
}

/*
 *	See if the given tty is able to transmit data without blocking. This
 *	may be done by checking the hardware, or if there is a software
 *	transmit queue by checking the queue is full.
 *
 *	There are three possible returns
 *	TTY_READY_NOW means fire away
 *	TTY_READY_SOON means we will spin trying until pre-empted. As the
 *		8bit processors are slow relative to baud rates it's often
 *		more efficient to do this
 *	TTY_READY_LATER means we will give up the CPU. This is best if the
 *		baud rate is low, the link is blocked by flow control signals
 *		or the CPU is fast.
 *
 *	If TTY_READY_LATER is returned then the kernel will also call
 *	tty_sleeping(minor) before sleeping on the tty so that the driver
 *	can turn on or off tx complete interrupts.
 *
 *	A video display that never blocks will just return TTY_READY_NOW
 */
uint_fast8_t tty_writeready(uint_fast8_t minor)
{
	/* FIXME: flow control */
	if (ttymap[minor] == 1)
		return (uart_lsr & 0x20) ? TTY_READY_NOW : TTY_READY_SOON;
	return prop_tty_writeready();
}

/*
 *	Write a character to a tty. This is the normal user space path for
 *	each outbound byte. It gets called in the normal tty flow, but may
 *	also be called from an interrupt to echo characters even if the
 *	tty is busy. This one reason to implement a small transmit queue.
 *
 *	If the character echo doesn't fit just drop it. It should pretty much
 *	never occur and there is nothing else to do.
 */
void tty_putc(uint_fast8_t minor ,uint_fast8_t c)
{
	if (ttymap[minor] == 1)
		uart_tx = c;
	else
		prop_tty_write(c);
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

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 *
 *	That needs tidying up in many platforms and we also need a proper way
 *	to say 'this port is fixed config' before making it so.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;
	if (ttymap[minor] == 1) {
		/* 16x50. Can actually be configured */
		d = 0x80;	/* DLAB (so we can write the speed) */
		d |= (t->c_cflag & CSIZE) >> 4;
		if(t->c_cflag & CSTOPB)
			d |= 0x04;
		if (t->c_cflag & PARENB)
			d |= 0x08;
		if (!(t->c_cflag & PARODD))
			d |= 0x10;
		uart_lcr = d;
		w = clocks[t->c_cflag & CBAUD];
		uart_ls = w;
		uart_ms = w >> 8;
		uart_lcr = d & 0x7F;
		/* FIXME: CTS/RTS support */
		d = 0x03;	/* DTR RTS */
		uart_mcr = d;
		uart_ier = 0x0D;	/* We don't use tx ints */
	}
}

/*
 *	This function is called when the kernel is about to sleep on a tty.
 *	We don't care about this.
 */
void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

/*
 *	Return 1 if the carrier on the terminal is raised. If the port has
 *	no carrier signal always return 1. It is used to block a port on open
 *	until carrier.
 */
int tty_carrier(uint_fast8_t minor)
{
        if (ttymap[minor] == 1)
		return uart_msr & 0x80;
	return 1;
}

/*
 *	When the input queue is part drained this method is called from the
 *	kernel so that hardware flow control signals can be updated.
 */
void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/*
 *	Our platform specific code so we have a function to call to poll the
 *	serial ports for activity.
 */
void tty_poll(void)
{	
	uint8_t msr;
	uint8_t minor = ttymap[1];	/* UART minor number */

	/* Should be IRQ driven but we might not be so poll anyway if
	   pending. IRQs are off here so this is safe */
	if (uart_lsr & 0x01)
		tty_inproc(minor, uart_rx);
	msr = uart_msr;
	/* If we have a 10Hz clock wired to DSR then do timer interrupts */
	if (timermsr && (msr & 0x02))
		timer_interrupt();
	/* DCD changed - tell the kernel so it can hangup or open ports */
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	/* TODO: CTS/RTS */

	/* Now as the PropIO driver to poll its input */
	prop_tty_poll(ttymap[2]);
}
