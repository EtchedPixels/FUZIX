/* origin: FreeBSD /usr/src/lib/msun/src/e_acosh.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */
/* acosh(x)
 * Method :
 *      Based on
 *              acosh(x) = log [ x + sqrt(x*x-1) ]
 *      we have
 *              acosh(x) := log(x)+ln2, if x is large; else
 *              acosh(x) := log(2x-1/(sqrt(x*x-1)+x)) if x>2; else
 *              acosh(x) := log1p(t+sqrt(2.0*t+t*t)); where t=x-1.
 *
 * Special cases:
 *      acosh(x) is NaN with signal if x<1.
 *      acosh(NaN) is NaN without signal.
 */

#include <math.h>
#include "libm.h"

static const double
ln2 = 6.93147180559945286227e-01; /* 0x3FE62E42, 0xFEFA39EF */

double acosh(double x)
{
	double t;
	int32_t hx;
	uint32_t lx;

	EXTRACT_WORDS(hx, lx, x);
	if (hx < 0x3ff00000) {  /* x < 1 */
		return (x-x)/(x-x);
	} else if (hx >= 0x41b00000) {  /* x > 2**28 */
		if (hx >= 0x7ff00000)  /* x is inf of NaN */
			return x+x;
		return log(x) + ln2;   /* acosh(huge) = log(2x) */
	} else if (((hx-0x3ff00000) | lx) == 0) {
		return 0.0;            /* acosh(1) = 0 */
	} else if (hx > 0x40000000) {  /* 2**28 > x > 2 */
		t = x*x;
		return log(2.0*x - 1.0/(x+sqrt(t-1.0)));
	} else {                /* 1 < x < 2 */
		t = x-1.0;
		return log1p(t + sqrt(2.0*t+t*t));
	}
}
