/*
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
#include <z280.h>

/* Remember these are in I/O FExxxx range */
__sfr __at 0x12 uart_tsr;
__sfr __at 0x14 uart_rsr;
__sfr __at 0x16 uart_rdr;
__sfr __at 0x18 uart_tdr;

/*
 *	One buffer for each tty. For now just the console
 */
static unsigned char tbuf1[TTYSIZ];

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
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
};

/* Write to system console. This is the backend to all the kernel messages,
   kprintf(), panic() etc. */

void kputchar(char c)
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
uint8_t tty_writeready(uint8_t minor)
{
	uint8_t r, iob;
	used(minor);
	iob = io_bank_set(0xFE);
	r = uart_tsr & 0x01 ? TTY_READY_NOW : TTY_READY_SOON;
	io_bank_set(iob);
	return r;
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
void tty_putc(uint8_t minor, unsigned char c)
{
	uint8_t iob;
	used(minor);
	iob = io_bank_set(0xFE);
	uart_tdr = c;
	io_bank_set(iob);
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
void tty_setup(uint8_t minor, uint8_t flags)
{
	used(minor);
}

/*
 *	This function is called when the kernel is about to sleep on a tty.
 *	We don't care about this.
 */
void tty_sleeping(uint8_t minor)
{
	used(minor);
}

/*
 *	Return 1 if the carrier on the terminal is raised. If the port has
 *	no carrier signal always return 1. It is used to block a port on open
 *	until carrier.
 */
int tty_carrier(uint8_t minor)
{
	used(minor);
	return 1;
}

/*
 *	When the input queue is part drained this method is called from the
 *	kernel so that hardware flow control signals can be updated.
 */
void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/*
 *	Our platform specific code so we have a function to call to poll the
 *	serial ports for activity.
 */
void tty_poll(void)
{	
	uint8_t iob = io_bank_set(0xFE);
	/* Should be IRQ driven but we might not be so poll anyway if
	   pending. IRQs are off here so this is safe */
	if (uart_rsr & 0x10)
		tty_inproc(1, uart_rdr);
	io_bank_set(iob);
}
