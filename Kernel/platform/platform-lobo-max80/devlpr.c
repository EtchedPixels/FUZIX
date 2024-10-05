#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <lobo.h>

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
    if (minor) {
    	udata.u_error = ENODEV;
    	return 1;
    }
    return 0;
}

int lpr_close(uint_fast8_t minor)
{
    minor; // shut up compiler
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
	volatile uint8_t *lp = lobo_io + 0x7E8;
	uint16_t ct;
	uint_fast8_t reg;

	while (udata.u_done < udata.u_count) {
		/* Try and balance polling and sleeping */
		while (((reg = *lp) & 0xA0) != 0x20) {
		        if (reg & 0x50) {
		                if (udata.u_done == 0) {
		                        if (reg & 0x40)
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
		lobo_io[0x7E8] = ugetc(p++);
		udata.u_done++;
	}
	return udata.u_done;
}
