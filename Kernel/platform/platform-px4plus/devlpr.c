#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <devices.h>

__sfr __at 0x16 lpstat;
__sfr __at 0x17 lpdata;

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
    int c = udata.u_count;
    char *p = udata.u_base;
    minor; rawflag; flag; // shut up compiler

    while(udata.u_done < udata.u_count) {
        while ((lpstat & 0x3) == 0x1) {
            if (iopoll())
                return udata.u_done;
	}
	/* Printer on fire ? */
        if (lpstat & 0x2)
		return -EIO;
        lpdata = ugetc(p++);
        /* Strobe */
        mod_ioctrlr(1,1);
        mod_ioctrlr(0,1);
        udata.u_done++;
    }
    return udata.u_done;
}
