#include <math.h>
#include "libm.h"

int __fpclassify(double x)
{
	uint32_t lo,hi;
	EXTRACT_WORDS(hi,lo,x);

	int e = hi>>7 & 0x7ff;
	if (!e) {
		if (lo == 0 && (hi << 1) == 0)
			return FP_ZERO;
		return FP_SUBNORMAL;
	}
	if (e==0x7ff) {
		if (lo || hi & 0x000FFFFF)
			return FP_NAN;
		return FP_INFINITE;
	}
	return FP_NORMAL;
}
