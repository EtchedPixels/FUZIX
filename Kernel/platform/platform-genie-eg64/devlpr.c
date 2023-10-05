#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <trs80.h>

#define lp	*((volatile uint8_t *)0x37E8)

__sfr __at 0xFD vg_lp;

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
        if (trs80_model == VIDEOGENIE) {
            while (vg_lp & 0x80) {
                if (iopoll())
                    return udata.u_done;
            }
            /* FIXME: tidy up ugetc and sysio checks globally */
            vg_lp = ugetc(p++);
        } else {
            while (lp & 0x80) {
                if (iopoll())
                    return udata.u_done;
            }
            /* FIXME: tidy up ugetc and sysio checks globally */
            lp = ugetc(p++);
        }
        udata.u_done++;
    }
    return udata.u_done;
}
