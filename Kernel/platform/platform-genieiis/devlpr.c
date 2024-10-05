#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <genie.h>

#define lp	0x37E8

int lpr_open(uint8_t minor, uint16_t flag)
{
    minor; flag; // shut up compiler
    return 0;
}

int lpr_close(uint8_t minor)
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

int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    char *p = udata.u_base;
    minor; rawflag; flag; // shut up compiler

    while(udata.u_done < udata.u_count) {
        while (mmio_read(lp) & 0x80) {
            if (iopoll())
                return udata.u_done;
        }
        mmio_write(lp, ugetc(p++));
        udata.u_done++;
    }
    return udata.u_done;
}
