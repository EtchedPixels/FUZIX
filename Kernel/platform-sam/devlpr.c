#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

__sfr __at 232 lpdata;
__sfr __at 233 lpstrobe;
__sfr __at 233 lpbusy;
__sfr __at 234 lpmode;

int lpr_open(uint8_t minor, uint16_t flag)
{
	used(flag);

	if (minor) {
		udata.u_error = ENODEV;
		return -1;
	}
	lpmode = 0;
	return 0;
}

int lpr_close(uint8_t minor)
{
	used(minor);
	return 0;
}

/* FIXME: review strobe delay requirement */
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


int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	minor;
	rawflag;
	flag;

	while (udata.u_done < udata.u_count) {
		while (lpbusy) {
			if (iopoll())
				return udata.u_done;
		}
		lpdata = ugetc(udata.u_base++);
		lpstrobe = 1;
		nap();
		lpstrobe = 0;
		udata.u_done++;
	}
	return (-1);
}
