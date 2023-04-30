/*
  A simple line printer char driver.  
    the only minor number, for now, is 0: the DriveWire Printer.
*/


#include <kernel.h>
#include <version.h>
#include <drivewire.h>
#include <kdata.h>
#include <device.h>
#include <devlpr.h>
#include <devdw.h>


int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor) {
		udata.u_error = ENODEV;
		return -1;
	}
	return 0;
}

int lpr_close(uint_fast8_t minor)
{
	if (minor == 0)
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

	if (minor == 0) {
		while (p < pe) {
			if ((n = iopoll(pe - p)) != 0)
				return n;
			dw_lpr(ugetc(p++));
		}
	}
	return udata.u_count;
}
