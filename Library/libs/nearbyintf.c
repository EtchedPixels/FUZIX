/* From MUSL */

/*#include <fenv.h>*/
#include <math.h>

/* FIXCME: exceptions to be done */

float nearbyintf(float x)
{
#ifdef FE_INEXACT
	#pragma STDC FENV_ACCESS ON
	int e;

	e = fetestexcept(FE_INEXACT);
#endif
	x = rintf(x);
#ifdef FE_INEXACT
	if (!e)
		feclearexcept(FE_INEXACT);
#endif
	return x;
}
