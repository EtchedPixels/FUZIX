/*
 *	The tty port on the MBC is very very simplistic indeed. In fact
 *	excessively so as we may block the entire system on a write 8(
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
#include <mbc2.h>

/*
 *	One buffer for each tty
 */
static uint8_t tbuf1[TTYSIZ];
static uint8_t sleeping;
static uint8_t writeq;

/*
 *	TTY masks - define which bits can be changed for each port
 */

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
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
ttyready_t tty_writeready(uint_fast8_t minor)
{
	irqflags_t irq;
	uint8_t d;
	if (writeq)
		return TTY_READY_NOW;
	irq = di();
	out(opcode, OP_GET_TXBUF);
	writeq = in(opread);
	irqrestore(irq);
	if (writeq)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
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
void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();
	out(opcode, OP_SERIAL_TX);
	out(opwrite, c);
	if (writeq)
		writeq--;
	irqrestore(irq);
}

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	used(minor);
	used(flags);
	/* Nothing can be set currently on the MBC */
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
 *
 *	Not available.
 */
int tty_carrier(uint_fast8_t minor)
{
	used(minor);
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
	tty_inproc(1, in(ttyport));
}
