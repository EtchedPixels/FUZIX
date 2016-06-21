/* From MUSL */

#include <math.h>
#include "libm.h"

#define N_SIGN ((uint64_t)1<<63)

double nextafter(double x, double y)
{
	union dshape ux, uy;
	uint64_t ax, ay;
	int e;

	if (isnan(x) || isnan(y))
		return x + y;
	ux.value = x;
	uy.value = y;
	if (ux.bits == uy.bits)
		return y;
	ax = ux.bits & ~N_SIGN;
	ay = uy.bits & ~N_SIGN;
	if (ax == 0) {
		if (ay == 0)
			return y;
		ux.bits = (uy.bits & N_SIGN) | 1;
	} else if (ax > ay || ((ux.bits ^ uy.bits) & N_SIGN))
		ux.bits--;
	else
		ux.bits++;
	e = ux.bits >> 52 & 0x7ff;
	/* raise overflow if ux.value is infinite and x is finite */
	if (e == 0x7ff)
		FORCE_EVAL(x+x);
	/* raise underflow if ux.value is subnormal or zero */
	if (e == 0)
		FORCE_EVAL(x*x + ux.value*ux.value);
	return ux.value;
}
