/* origin: FreeBSD /usr/src/lib/msun/src/e_coshf.c */
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

static const float huge = 1.0e30;

float coshf(float x)
{
	float t, w;
	int32_t ix;

	GET_FLOAT_WORD(ix, x);
	ix &= 0x7fffffff;

	/* x is INF or NaN */
	if (ix >= 0x7f800000)
		return x*x;

	/* |x| in [0,0.5*ln2], return 1+expm1(|x|)^2/(2*exp(|x|)) */
	if (ix < 0x3eb17218) {
		t = expm1f(fabsf(x));
		w = 1.0f+t;
		if (ix<0x39800000)
			return 1.0f;  /* cosh(tiny) = 1 */
		return 1.0f + (t*t)/(w+w);
	}

	/* |x| in [0.5*ln2,9], return (exp(|x|)+1/exp(|x|))/2; */
	if (ix < 0x41100000) {
		t = expf(fabsf(x));
		return 0.5f*t + 0.5f/t;
	}

	/* |x| in [9, log(maxfloat)] return 0.5f*exp(|x|) */
	if (ix < 0x42b17217)
		return 0.5f*expf(fabsf(x));

	/* |x| in [log(maxfloat), overflowthresold] */
	if (ix <= 0x42b2d4fc)
		return __expo2f(fabsf(x));

	/* |x| > overflowthresold, cosh(x) overflow */
	return huge*huge;
}
