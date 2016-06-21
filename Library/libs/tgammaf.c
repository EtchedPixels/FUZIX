/* From MUSL */

#include <math.h>

// FIXME: use lanczos approximation

float tgammaf(float x)
{
	int sign;
	float y;

	y = exp(lgammaf_r(x, &sign));
	if (sign < 0)
		y = -y;
	return y;
}
