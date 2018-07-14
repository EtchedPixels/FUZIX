/* origin: FreeBSD /usr/src/lib/msun/src/e_asinf.c */
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
huge = 1.000e+30,
/* coefficients for R(x^2) */
pS0 =  1.6666586697e-01,
pS1 = -4.2743422091e-02,
pS2 = -8.6563630030e-03,
qS1 = -7.0662963390e-01;

static const double
pio2 = 1.570796326794896558e+00;

float asinf(float x)
{
	double s;
	float t,w,p,q;
	int32_t hx,ix;

	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;
	if (ix >= 0x3f800000) {  /* |x| >= 1 */
		if (ix == 0x3f800000)  /* |x| == 1 */
			return x*pio2;  /* asin(+-1) = +-pi/2 with inexact */
		return 0/(x-x);  /* asin(|x|>1) is NaN */
	} else if (ix < 0x3f000000) {  /* |x|<0.5 */
		if (ix < 0x39800000) {  /* |x| < 2**-12 */
			if (huge+x > 1.0f)
				return x; /* return x with inexact if x!=0 */
		}
		t = x*x;
		p = t*(pS0+t*(pS1+t*pS2));
		q = 1.0f+t*qS1;
		w = p/q;
		return x + x*w;
	}
	/* 1 > |x| >= 0.5 */
	w = 1.0f - fabsf(x);
	t = w*0.5f;
	p = t*(pS0+t*(pS1+t*pS2));
	q = 1.0f+t*qS1;
	s = sqrt(t);
	w = p/q;
	t = pio2-2.0*(s+s*w);
	if (hx > 0)
		return t;
	return -t;
}
