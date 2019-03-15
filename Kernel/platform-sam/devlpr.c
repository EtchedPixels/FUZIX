#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

/* FIXME: There are two comms interfaces

   1. The SPI printer port on 232/233 mode on 234 (supported here)
   2. The MGT comms interface. Printer port on 232/233 or 234/235 but no
      direction control, and 236-239 allow up to four SCC2691 UARTs

   We support ether interface in the default jumpering */

__sfr __at 232 lpdata;
__sfr __at 233 lpstrobe;
__sfr __at 233 lpbusy;
__sfr __at 234 lpmode;

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	used(flag);

	if (minor) {
		udata.u_error = ENODEV;
		return -1;
	}
	lpmode = 0;
	return 0;
}

int lpr_close(uint_fast8_t minor)
{
	used(minor);
	return 0;
}

/* The call is more than enough to ensure we make the parallel timing */
static void nap(void)
{
}

static uint8_t iopoll(void)
{
	/* Ought to be a core helper for this lot ? */
	if (need_reschedule())
		_sched_yield();
	if (chksigs()) {
		if (!udata.u_done) {
			udata.u_error = EINTR;
			udata.u_done = (usize_t) - 1;
		}
		return 1;
	}
	return 0;
}


int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	minor;
	rawflag;
	flag;

	while (udata.u_done < udata.u_count) {
		while (lpbusy & 1) {
			if (iopoll())
				return udata.u_done;
		}
		lpdata = ugetc(udata.u_base++);
		lpstrobe = 1;
		nap();
		lpstrobe = 0;
		udata.u_done++;
	}
	return udata.u_done;
}
