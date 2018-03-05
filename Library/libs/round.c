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

#include "libm.h"

double round(double x)
{
	/* Most significant word, least significant word. */
	int32_t msw, exponent_less_1023;
	uint32_t lsw;

	EXTRACT_WORDS(msw, lsw, x);

	/* Extract exponent field. */
	exponent_less_1023 = ((msw & 0x7ff00000UL) >> 20) - 1023;

	if (exponent_less_1023 < 20) {
		if (exponent_less_1023 < 0) {
			msw &= 0x80000000UL;
			if (exponent_less_1023 == -1)
				/* Result is +1.0 or -1.0. */
				msw |= (1023UL << 20);
			lsw = 0;
		} else {
			uint32_t exponent_mask = 0x000fffffUL >> exponent_less_1023;
			if ((msw & exponent_mask) == 0 && lsw == 0)
				/* x in an integral value. */
				return x;

			msw += 0x00080000 >> exponent_less_1023;
			msw &= ~exponent_mask;
			lsw = 0;
		}
	} else if (exponent_less_1023 > 51) {
		if (exponent_less_1023 == 1024)
			/* x is NaN or infinite. */
			return x + x;
		else
			return x;
	} else {
		uint32_t exponent_mask = 0xffffffff >> (exponent_less_1023 - 20);
		uint32_t tmp;

		if ((lsw & exponent_mask) == 0)
			/* x is an integral value. */
			return x;

		tmp = lsw + (1 << (51 - exponent_less_1023));
		if (tmp < lsw)
			msw += 1;
		lsw = tmp;

		lsw &= ~exponent_mask;
	}
	INSERT_WORDS(x, msw, lsw);

	return x;
}
