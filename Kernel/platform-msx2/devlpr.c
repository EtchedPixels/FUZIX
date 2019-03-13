#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

__sfr __at 0x90 lpstat;
__sfr __at 0x91 lpdata;

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

int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	int c = udata.u_count;
	char *p = udata.u_base;
	uint16_t ct;

	minor;
	rawflag;
	flag;			// shut up compiler

	while (c-- > 0) {
		ct = 0;

		/* Try and balance polling and sleeping */
		while (lpstat & 2) {
			ct++;
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
		/* Data */
		lpdata = ugetc(p++);
		/* Strobe */
		lpstat |= 1;
		lpstat &= ~1;
	}
	return udata.u_count;
}
