/* origin: FreeBSD /usr/src/lib/msun/src/s_cosf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
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

/* Small multiples of pi/2 rounded to double precision. */
static const double
c1pio2 = __1M_PI_2, /* 0x3FF921FB, 0x54442D18 */
c2pio2 = __2M_PI_2, /* 0x400921FB, 0x54442D18 */
c3pio2 = __3M_PI_2, /* 0x4012D97C, 0x7F3321D2 */
c4pio2 = __4M_PI_2; /* 0x401921FB, 0x54442D18 */

float cosf(float x)
{
	double y;
	int32_t n, hx, ix;

	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;
	if (ix <= 0x3f490fda) {  /* |x| ~<= pi/4 */
		if (ix < 0x39800000)  /* |x| < 2**-12 */
			if ((int)x == 0)  /* raise inexact if x != 0 */
				return 1.0;
		return __cosdf(x);
	}
	if (ix <= 0x407b53d1) {  /* |x| ~<= 5*pi/4 */
		if (ix > 0x4016cbe3)  /* |x|  ~> 3*pi/4 */
			return -__cosdf(hx > 0 ? x-c2pio2 : x+c2pio2);
		else {
			if (hx > 0)
				return __sindf(c1pio2 - x);
			else
				return __sindf(x + c1pio2);
		}
	}
	if (ix <= 0x40e231d5) {  /* |x| ~<= 9*pi/4 */
		if (ix > 0x40afeddf)  /* |x| ~> 7*pi/4 */
			return __cosdf(hx > 0 ? x-c4pio2 : x+c4pio2);
		else {
			if (hx > 0)
				return __sindf(x - c3pio2);
			else
				return __sindf(-c3pio2 - x);
		}
	}

	/* cos(Inf or NaN) is NaN */
	if (ix >= 0x7f800000)
		return x-x;

	/* general argument reduction needed */
	n = __rem_pio2f(x,&y);
	switch (n&3) {
	case 0: return  __cosdf(y);
	case 1: return  __sindf(-y);
	case 2: return -__cosdf(y);
	default:
		return  __sindf(y);
	}
}
