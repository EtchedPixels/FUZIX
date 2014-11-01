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

int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    int c = udata.u_count;
    char *p = udata.u_base;
    uint16_t ct;
    minor; rawflag; flag; // shut up compiler

    while(c-- > 0) {
	ct = 0;
        while ((lpstat & 0x3) == 0x1) {
		ct++;
		/* Try and be polite */
		if (ct == 10000) {
			udata.u_ptab->p_timeout = 3;
			if (psleep_flags(NULL, flag)) {
				if (udata.u_count)
					udata.u_error = 0;
				return udata.u_count;
			}
			ct = 0;
		}
	}
	/* Printer on fire ? */
        if (lpstat & 0x2)
		return -EIO;
        lpdata = ugetc(p++);
        /* Strobe */
        mod_ioctrlr(1,1);
        mod_ioctrlr(0,1);
    }
    return (-1);
}
