/* origin: FreeBSD /usr/src/lib/msun/src/e_atanhf.c */
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

static const float huge = 1e30;

float atanhf(float x)
{
	float t;
	int32_t hx,ix;

	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;
	if (ix > 0x3f800000)                   /* |x| > 1 */
		return (x-x)/(x-x);
	if (ix == 0x3f800000)
		return x / 0.0;
	if (ix < 0x31800000 && huge+x > 0.0f)  /* x < 2**-28 */
		return x;
	SET_FLOAT_WORD(x, ix);
	if (ix < 0x3f000000) {                 /* x < 0.5 */
		t = x+x;
		t = 0.5f*log1pf(t + t*x/(1.0f-x));
	} else
		t = 0.5f*log1pf((x+x)/(1.0f-x));
	if (hx >= 0)
		return t;
	return -t;
}
