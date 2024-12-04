/*
 *	Generic printer interface
 *
 *	The caller needs to define the following in the config (and they can
 *	be defined to variables if needed)
 *
 *	CONFIG_LP_GENERIC
 *	LP_DATA
 *	LP_STATUS
 *	LP_IS_BUSY		- eg to (lpstate & 0x80)
 *	LP_STROBE		- these may be null for auto-strobe
 *				- delays need to be in the macro
 *				- may reference lpdata if needed
 */
#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <lpgeneric.h>

#ifdef CONFIG_LP_GENERIC

__sfr __banked __at LP_STATUS lpstat;
__sfr __banked __at LP_DATA lpdata;

int lpr_open(uint8_t minor, uint16_t flag)
{
    used(flag);

    if (minor) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int lpr_close(uint8_t minor)
{
    used(minor);
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
    minor; rawflag; flag; /* shut up compiler */

    while(udata.u_done < udata.u_count) {
        while (LP_IS_BUSY) {
            if (iopoll())
                return udata.u_done;
        }
        lpdata = ugetc(p++);
        LP_STROBE;
        udata.u_done++;
    }
    return udata.u_done;
}

#endif
