/* origin: FreeBSD /usr/src/lib/msun/src/e_atanh.c */
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
/* atanh(x)
 * Method :
 *    1.Reduced x to positive by atanh(-x) = -atanh(x)
 *    2.For x>=0.5
 *                  1              2x                          x
 *      atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *                  2             1 - x                      1 - x
 *
 *      For x<0.5
 *      atanh(x) = 0.5*log1p(2x+2x*x/(1-x))
 *
 * Special cases:
 *      atanh(x) is NaN if |x| > 1 with signal;
 *      atanh(NaN) is that NaN with no signal;
 *      atanh(+-1) is +-INF with signal.
 *
 */

#include <math.h>
#include "libm.h"

static const double huge = 1e300;

double atanh(double x)
{
	double t;
	int32_t hx,ix;
	uint32_t lx;

	EXTRACT_WORDS(hx, lx, x);
	ix = hx & 0x7fffffff;
	if ((ix | ((lx|-lx)>>31)) > 0x3ff00000)  /* |x| > 1 */
		return (x-x)/(x-x);
	if (ix == 0x3ff00000)
		return x/0.0;
	if (ix < 0x3e300000 && (huge+x) > 0.0)   /* x < 2**-28 */
		return x;
	SET_HIGH_WORD(x, ix);
	if (ix < 0x3fe00000) {                   /* x < 0.5 */
		t = x+x;
		t = 0.5*log1p(t + t*x/(1.0-x));
	} else
		t = 0.5*log1p((x+x)/(1.0-x));
	if (hx >= 0)
		return t;
	return -t;
}
