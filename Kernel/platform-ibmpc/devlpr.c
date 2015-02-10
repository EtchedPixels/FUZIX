#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <device.h>
#include <devlpr.h>

/* random test places */
uint8_t *lpstat = (uint8_t *)0xFF00;
uint8_t *lpdata = (uint8_t *)0xFF01;

int lpr_open(uint8_t minor, uint16_t flag)
{
	minor;
	flag;			// shut up compiler
	return 0;
}

int lpr_close(uint8_t minor)
{
	minor;			// shut up compiler
	return 0;
}

int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
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
		while (*lpstat & 2) {
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
		*lpdata = ugetc(p++);
	}
	return udata.u_count;
}
