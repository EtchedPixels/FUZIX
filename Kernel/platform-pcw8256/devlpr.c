#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <printer.h>
#include <pcw8256.h>

/*
 *	We have so many different printer interfaces to chooose between
 *
 *	Built in
 *	- Smart dot matrix interface (needs to be fed specific commands)
 *	- PCW9512 Smart daisy wheel interface (needs custom interface)
 *	- PCW9512 centronics
 *
 *	Add on
 *	- CPS8256
 *	- Centronics
 *
 *	The smart daisy wheel has a complex bidirectional API that doesn't
 *	fit /dev/lp at all so we don't support it yet.
 */

#define MAX_LP 4

int lpr_open(uint8_t minor, uint16_t flag)
{
	used(flag);
	if (minor >= MAX_LP) {
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
			udata.u_done = (usize_t) - 1;
		}
		return 1;
	}
	return 0;
}

__sfr __at 0xFC lpc;
__sfr __at 0xFD lpd;

__sfr __banked __at 0x00FC lpc512;
__sfr __banked __at 0x01FC lpc512c;
__sfr __banked __at 0x00FD lpc512r;
__sfr __banked __at 0x01FD lpc512s;

__sfr __at 0x84 cens;
__sfr __at 0x85 cends;
__sfr __at 0x87 cend;

__sfr __at 0xE8 cps_par;

static uint8_t lp512ce;

static uint8_t lpbusy(uint8_t minor)
{
	uint8_t r;
	switch(minor) {
	case 0:
		/* Dot matrix */
		if (lpd & 0x02)
			return 1;
		return 0;
	case 1:
		/* PCW512 style centronics */
		/* Controller busy */
		if (lpc512r & 0x02)
			return 1;
		/* Poll status */
		lpc512c = 0x02;
		lpc512 = 0x00;
		while(!(lpc512r & 0x01));
		r = lpc512r;
		lp512ce = r;
		/* Return not busy for errors */
		return !!(r & 0x68);
	case 2:
		/* CPS8256 */
		return !!cps_centronics_busy();
	case 3:
	default:
		return cens & 1;
	}
}

static void lpsend(uint8_t minor, uint8_t c)
{
	switch(minor) {
	case 0:
		lpd = c;
		break;
	case 1:
		while(!(lpd & 0x02));
		lpc512c = 0x04;
		lpc512 = c;
		break;
	case 2:
		cps_par = c;
		cps_centronics_strobe(c);
		break;
	case 3:
		cend = c;
		cends = c;
		cend = c;
		break;
	}
}

static uint8_t lperror(uint8_t minor)
{
	uint8_t r;
	switch(minor) {
	case 0:
		if ((lpd & 0x01) && (lpc != 0xF8))
			return 1;
		return 0;
	case 1:
		r = lp512ce;
		lp512ce = 0;
		if (r & 0x28)
			return 1;
		return 0;
	default:
		return 0;
	}
}

int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	char *p = udata.u_base;

	used(rawflag);
	used(flag);			// shut up compiler

	while (udata.u_done < udata.u_count) {
		if (lpbusy(minor)) {
			if (iopoll())
				return udata.u_done;
		}
		if (lperror(minor)) {
			if (udata.u_done)
				break;
			udata.u_error = EIO;
			return -1;
		}
		/* Data */
		lpsend(minor, ugetc(p++));
		udata.u_done++;
	}
	return udata.u_done;
}

int lpr_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	uint8_t s, r;

	used(ptr);

	if (arg == LPIOCGSDATA) {
		if (minor == 0)
			return lpd;
	}
	if (arg == LPIOCGSERR) {
		if (minor == 0)
			return lpc;
	}
	if (arg == LPIOCSTAT && minor == 1) {
		while(lpc512r & 0x02);
		/* Poll status */
		lpc512c = 0x02;
		lpc512 = 0x00;
		while(!(lpc512r & 0x01));
		s = lpc512r;
		r = 0;
		if (s & 0x80)
			r = LP_ACK;
		if (s & 0x40)
			r |= LP_BUSY;
		if (s & 0x20)
			r |= LP_ERROR;
		if (s & 0x10)
			r |= LP_SELECT;
		if (s & 0x08)
			r |= LP_PAPER;
		return r;
	}
	return -1;
}
