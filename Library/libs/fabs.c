/* From MUSL */

#include "math.h"
#include "libm.h"

double fabs(double x)
{
	uint32_t hi;

	GET_HIGH_WORD(x, hi);
	hi &= 0x7FFFFFFFUL;
	SET_HIGH_WORD(x, hi);
	return x;
}
