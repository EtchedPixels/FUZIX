/* From MUSL */

#include <math.h>
#include "libm.h"

#define N_SIGN 0x80000000

float nextafterf(float x, float y)
{
	union fshape ux, uy;
	uint32_t ax, ay, e;

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
	e = ux.bits & 0x7f800000;
	/* raise overflow if ux.value is infinite and x is finite */
	if (e == 0x7f800000) {
		volatile float dummy = x + x;
	}
	/* raise underflow if ux.value is subnormal or zero */
	if (e == 0) {
		volatile float dummy = x*x + ux.value*ux.value;
	}
	return ux.value;
}
