/* From MUSL */

#include <math.h>

double lgamma(double x)
{
	return lgamma_r(x, &signgam);
}
