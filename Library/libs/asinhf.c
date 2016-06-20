/* origin: FreeBSD /usr/src/lib/msun/src/s_asinhf.c */
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
#include "libm.h"

static const float
ln2 = 6.9314718246e-01, /* 0x3f317218 */
huge= 1.0000000000e+30;

float asinhf(float x)
{
	float t,w;
	int32_t hx,ix;

	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;
	if (ix >= 0x7f800000)   /* x is inf or NaN */
		return x+x;
	if (ix < 0x31800000) {  /* |x| < 2**-28 */
		/* return x inexact except 0 */
		if (huge+x > 1.0f)
			return x;
	}
	if (ix > 0x4d800000) {  /* |x| > 2**28 */
		w = logf(fabsf(x)) + ln2;
	} else if (ix > 0x40000000) {  /* 2**28 > |x| > 2.0 */
		t = fabsf(x);
		w = logf(2.0f*t + 1.0f/(sqrtf(x*x+1.0f)+t));
	} else {                /* 2.0 > |x| > 2**-28 */
		t = x*x;
		w =log1pf(fabsf(x) + t/(1.0f+sqrtf(1.0f+t)));
	}
	if (hx > 0)
		return w;
	return -w;
}
