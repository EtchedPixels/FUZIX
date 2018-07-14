/* origin: FreeBSD /usr/src/lib/msun/src/e_acoshf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <math.h>
#include <signal.h>
#include "libm.h"

static const float
ln2 = 6.9314718246e-01; /* 0x3f317218 */

float acoshf(float x)
{
	float t;
	int32_t hx;

	GET_FLOAT_WORD(hx, x);
	if (hx < 0x3f800000)  /* x < 1 */
		return (x-x)/(x-x);
	else if (hx >= 0x4d800000) {  /* x > 2**28 */
		if (hx >= 0x7f800000)  /* x is inf of NaN */
			return x + x;
		return logf(x) + ln2;  /* acosh(huge)=log(2x) */
	} else if (hx == 0x3f800000) {
		return 0.0f;  /* acosh(1) = 0 */
	} else if (hx > 0x40000000) {  /* 2**28 > x > 2 */
		t = x*x;
		return logf(2.0f*x - 1.0f/(x+sqrtf(t-1.0f)));
	} else {                /* 1 < x < 2 */
		t = x-1.0f;
		return log1pf(t + sqrtf(2.0f*t+t*t));
	}
}
