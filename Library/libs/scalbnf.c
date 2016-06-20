/* From MUSL */

#include "libm.h"

float scalbnf(float x, int n)
{
	float scale;

	if (n > 127) {
		x *= 0x1p127f;
		n -= 127;
		if (n > 127) {
			x *= 0x1p127f;
			n -= 127;
			if (n > 127) {
				x = (float)(x * 0x1p127f);
				return x;
			}
		}
	} else if (n < -126) {
		x *= 0x1p-126f;
		n += 126;
		if (n < -126) {
			x *= 0x1p-126f;
			n += 126;
			if (n < -126) {
				x = (float)(x * 0x1p-126f);
				return x;
			}
		}
	}
	SET_FLOAT_WORD(scale, (uint32_t)(0x7f+n)<<23);
	x = (float)(x * scale);
	return x;
}
