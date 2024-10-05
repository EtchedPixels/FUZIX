#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

#define lpstat	0x02		/* I/O 2 and 3 */
#define lpdata	0x03

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
    minor; flag; // shut up compiler
    return 0;
}

int lpr_close(uint_fast8_t minor)
{
    minor; // shut up compiler
    return 0;
}

static uint_fast8_t iopoll(void)
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
    minor; rawflag; flag; // shut up compiler

    while(udata.u_done < udata.u_count) {
        /* Note; on real hardware it might well be necessary to
           busy wait a bit just to get acceptable performance */
        while (in(lpstat) != 0xFF) {
            if (iopoll())
                return udata.u_done;
        }
        /* FIXME: tidy up ugetc and sysio checks globally */
        out(lpdata, ugetc(p++));
        udata.u_done++;
    }
    return udata.u_done;
}
