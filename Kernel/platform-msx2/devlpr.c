#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

__sfr __at 0x90 lpstat;
__sfr __at 0x91 lpdata;

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	minor;
	flag;			// shut up compiler
	return 0;
}

int lpr_close(uint_fast8_t minor)
{
	minor;			// shut up compiler
	return 0;
}

static uint8_t iopoll(void)
{
	/* Ought to be a core helper for this lot ? */
	if (need_reschedule())
		_sched_yield();
	if (chksigs()) {
		if (!udata.u_done) {
			udata.u_error = EINTR;
			udata.u_done = (usize_t)-1;
                }
                return 1;
	}
	return 0;
}

int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	char *p = udata.u_base;

	minor;
	rawflag;
	flag;			// shut up compiler

	while (udata.u_done < udata.u_count) {
		/* Try and balance polling and sleeping */
		while (lpstat & 2) {
			if (iopoll())
				return udata.u_done;
		}
		/* Data */
		lpdata = ugetc(p++);
		/* Strobe */
		lpstat |= 1;
		lpstat &= ~1;
		udata.u_done++;
	}
	return udata.u_done;
}
