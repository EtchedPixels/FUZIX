#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <bios.h>

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	used(flag);
	if (minor < biosinfo->num_lpt)
		return 0;
	udata.u_error = ENODEV;
	return -1;
}

int lpr_close(uint_fast8_t minor)
{
	used(minor);
	return 0;
}

static int iopoll(void)
{
	/* Ought to be a core helper for this lot ? */
	if (need_reschedule())
		_sched_yield();
	if (chksigs()) {
		if (udata.u_done)
			return udata.u_done;
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
	uint8_t c;

	used(rawflag);
	used(flag);

	/* O_NDELAY ? */

	while (udata.u_done < udata.u_count) {
		/* Printer busy ? */
		while (fuzixbios_lpt_busy(minor)) {
			if ((n = iopoll()) != 0)
				return n;
		}
		c = ugetc(p++);
		c = fuzixbios_lpt_tx(minor | (c << 8));
		if (c) {
			if (udata.u_done)
				break;
			udata.u_error = c;
			return -1;
		}
		udata.u_done++;
	}
	return udata.u_done;
}
