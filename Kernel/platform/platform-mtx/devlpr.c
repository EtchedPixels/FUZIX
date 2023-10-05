#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

__sfr __at 0x00 lpstrobel;
__sfr __at 0x04 lpdata;		/* Data on write, status+strobe clr on read */

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
	uint16_t ct;
	uint8_t reg;

	minor;
	rawflag;
	flag;			// shut up compiler

	while (udata.u_done < udata.u_count) {

		/* Try and balance polling and sleeping */
		while ((reg = lpdata) & 1) {
		        reg ^= 2;
		        if (reg & 6) {
		                if (udata.u_done == 0) {
		                        if (reg & 4)
                		                udata.u_error = ENOSPC;
                                        else
                		                udata.u_error = EIO;
					return -1;
                                }
                                return udata.u_done;
                        }
                        if (iopoll())
                        	return udata.u_done;
		}
		/* Data */
		lpdata = ugetc(p++);
		/* Strobe - FIXME: should be 1uS */
		for (reg = 0; reg < 128; reg++);
		reg = lpstrobel;
		for (reg = 0; reg < 128; reg++);
		/* Strobe back high */
		reg = lpdata;
		udata.u_done++;
	}
	return udata.u_done;
}
