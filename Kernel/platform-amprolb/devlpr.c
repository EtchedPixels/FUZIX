#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

__sfr __at 0x01	lp;
__sfr __at 0x02 lpss;
__sfr __at 0x03 lpsc;
__sfr __at 0x8C siob_c;

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	if (!minor)
		return 0;
	udata.u_error = ENODEV;
	return -1;
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

static uint8_t lpstat(void)
{
	uint8_t r;
	irqflags_t irq = di();
	siob_c = 0x10;
	r = siob_c & 0x10;
	irqrestore(irq);
	return r;
}

int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	uint8_t *p = udata.u_base;
	uint8_t *pe = p + udata.u_count;
	int n;

	while (p < pe) {
		/* Printer busy ? */
		while (!lpstat()) {
			if ((n = iopoll(pe - p)) != 0)
				return n;
		}
		lp = ugetc(p++);
		lpss = 1;
		lpsc = 1;
	}
	return pe - p;
}
