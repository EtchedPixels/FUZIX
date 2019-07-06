#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <device.h>
#include <devlpr.h>

volatile uint8_t * const pia0b = (uint8_t *)0xFF02;
volatile uint8_t * const pia1a = (uint8_t *)0xFF20;
volatile uint8_t * const pia1b = (uint8_t *)0xFF22;

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor < 2)
		return 0;
	udata.u_error = ENODEV;
	return -1;
}

int lpr_close(uint_fast8_t minor)
{
	if (minor == 1)
		dw_lpr_close();
	return 0;
}

static int iopoll(int sofar)
{
	/* Ought to be a core helper for this lot ? */
	if (need_reschedule())
		_sched_yield();
	if (chksigs()) {
		if (sofar)
			return sofar;
		udata.u_error = EINTR;
		return -1;
	}
	return 0;
}

int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	uint8_t *p = udata.u_base;
	uint8_t *pe = p + udata.u_count;
	int n;
	irqflags_t irq;

	if (minor == 1) {
		while (p < pe) {
			if ((n = iopoll(pe - p)) != 0)
				return n;
			dw_lpr(ugetc(p++));
		}
	} else {
		while (p < pe) {
			/* Printer busy ? */
			while (*pia1b & 1) {
				if ((n = iopoll(pe - p)) != 0)
					return n;
			}
			/* The Dragon shares the printer and keyboard scan
			   lines. Make sure we don't collide */
			irq = di();
			*pia0b = ugetc(p++);
			*pia1a |= 0x02;	/* Strobe */
			*pia1a &= ~0x02;
			irqrestore(irq);
		}
	}
	return pe - p;
}
