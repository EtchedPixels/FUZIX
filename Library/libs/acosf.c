/* origin: FreeBSD /usr/src/lib/msun/src/e_acosf.c */
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
pi      = 3.1415925026e+00, /* 0x40490fda */
pio2_hi = 1.5707962513e+00; /* 0x3fc90fda */
static const volatile float
pio2_lo = 7.5497894159e-08; /* 0x33a22168 */
static const float
pS0 =  1.6666586697e-01,
pS1 = -4.2743422091e-02,
pS2 = -8.6563630030e-03,
qS1 = -7.0662963390e-01;

float acosf(float x)
{
	float z,p,q,r,w,s,c,df;
	int32_t hx,ix;

	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;
	if (ix >= 0x3f800000) {  /* |x| >= 1 */
		if (ix == 0x3f800000) {  /* |x| == 1 */
			if (hx > 0) return 0.0f;  /* acos(1) = 0 */
			return pi + 2.0f*pio2_lo;  /* acos(-1)= pi */
		}
		return (x-x)/(x-x);  /* acos(|x|>1) is NaN */
	}
	if (ix < 0x3f000000) {   /* |x| < 0.5 */
		if (ix <= 0x32800000) /* |x| < 2**-26 */
			return pio2_hi + pio2_lo;
		z = x*x;
		p = z*(pS0+z*(pS1+z*pS2));
		q = 1.0f+z*qS1;
		r = p/q;
		return pio2_hi - (x - (pio2_lo-x*r));
	} else if (hx < 0) {     /* x < -0.5 */
		z = (1.0f+x)*0.5f;
		p = z*(pS0+z*(pS1+z*pS2));
		q = 1.0f+z*qS1;
		s = sqrtf(z);
		r = p/q;
		w = r*s-pio2_lo;
		return pi - 2.0f*(s+w);
	} else {                 /* x > 0.5 */
		int32_t idf;

		z = (1.0f-x)*0.5f;
		s = sqrtf(z);
		df = s;
		GET_FLOAT_WORD(idf,df);
		SET_FLOAT_WORD(df,idf&0xfffff000);
		c  = (z-df*df)/(s+df);
		p = z*(pS0+z*(pS1+z*pS2));
		q = 1.0f+z*qS1;
		r = p/q;
		w = r*s+c;
		return 2.0f*(df+w);
	}
}
