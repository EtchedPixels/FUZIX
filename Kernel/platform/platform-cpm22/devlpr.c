#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <cpm.h>

int lpr_open(uint8_t minor, uint16_t flag)
{
    flag; // shut up compiler
    if (minor) {
        udata.u_error = ENXIO;
        return -1;
    }
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

    /* FIXME: update asm glue and pass printer # */
    while(udata.u_done < udata.u_count) {
        while (!cpm_listst()) {
            if (iopoll())
                return udata.u_done;
        }
        cpm_list(ugetc(p++));
        udata.u_done++;
    }
    return udata.u_done;
}
