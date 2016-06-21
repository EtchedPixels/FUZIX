/* origin: FreeBSD /usr/src/lib/msun/src/e_sinhf.c */
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

#include "math.h"
#include "libm.h"

static const float huge = 1.0e37;

float sinhf(float x)
{
	float t, h;
	int32_t ix, jx;

	GET_FLOAT_WORD(jx, x);
	ix = jx & 0x7fffffff;

	/* x is INF or NaN */
	if (ix >= 0x7f800000)
		return x + x;

	h = 0.5;
	if (jx < 0)
		h = -h;
	/* |x| in [0,9], return sign(x)*0.5*(E+E/(E+1))) */
	if (ix < 0x41100000) {   /* |x|<9 */
		if (ix < 0x39800000)  /* |x|<2**-12 */
			/* raise inexact, return x */
			if (huge+x > 1.0f)
				return x;
		t = expm1f(fabsf(x));
		if (ix < 0x3f800000)
			return h*(2.0f*t - t*t/(t+1.0f));
		return h*(t + t/(t+1.0f));
	}

	/* |x| in [9, logf(maxfloat)] return 0.5*exp(|x|) */
	if (ix < 0x42b17217)
		return h*expf(fabsf(x));

	/* |x| in [logf(maxfloat), overflowthresold] */
	if (ix <= 0x42b2d4fc)
		return h * 2.0f * __expo2f(fabsf(x)); /* h is for sign only */

	/* |x| > overflowthresold, sinh(x) overflow */
	return x*huge;
}
