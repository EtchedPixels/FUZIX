#include <math.h>

float lgammaf(float x)
{
	return lgammaf_r(x, &signgam);
}
