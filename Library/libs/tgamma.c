/* From MUSL */

#include <math.h>

// FIXME: use lanczos approximation

double tgamma(double x)
{
	int sign;
	double y;

	y = exp(lgamma_r(x, &sign));
	if (sign < 0)
		y = -y;
	return y;
}
