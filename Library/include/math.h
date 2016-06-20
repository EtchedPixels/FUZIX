#ifndef _MATH_H
#define _MATH_H

#ifndef double

/* Compiler with full float/double support */

extern double fabs(double);
extern double floor(double);
extern double ceil(double);
extern double modf(double, double *);
extern double frexp(double, int *);
extern double ldexp(double, int);
extern double atof(char *);

extern double sqrt(double);

extern double sin(double);
extern double cos(double);
extern double tan(double);
extern double asin(double);
extern double acos(double);
extern double atan(double);
extern double atan2(double, double);
extern double sinh(double);
extern double cosh(double);
extern double tanh(double);

extern double log(double);
extern double log10(double);
extern double log1p(double);
extern double log2(double);
extern double logb(double);

extern double nan(const char *__tagp);

extern double pow(double, double);
extern double exp(double);

extern double scalbln(double, long);
extern double scalbn(double, int);

extern double hypot(double, double);

#else

/* We have double defined as float .. so fix up the support routines */
#define fabs(a) fabsf(a)
#define floor(a) floorf(a)
#define ceil(a) ceilf(a)
#define modf(a,b) modff(a,b)
#define frexp(a,b) frexpf(a,b)
#define ldexp(a,b) ldexpf(a,b)
/* FIXME atof equivalence */

#define sqrt(a) sqrtf(a)

#define sin(a) sinf(a)
#define cos(a) cosf(a)
#define tan(a) tanf(a)
#define asin(a) asinf(a)
#define acos(a) acosf(a)
#define atan(a) atanf(a)
#define atan2(a,b) atan2f(a,b)
#define sinh(a)	sinhf(a)
#define cosh(a)	coshf(a)
#define tanh(a) tanhf(a)

#define log(a)		logf(a)
#define log10(a)	log10f(a)
#define log1p(a)	log1pf(a)
#define log2(a)		log2f(a)
#define logb(a)		logbf(a)

#define nan(a)		nanf(a)

#define pow(a,b)	powf(a,b)
#define exp(a)		expf(a)
#define hypot(a,b)	hypotf(a,b)

#define scalbln(a,b)	scalblnf(a,b)
#define scalbn(a,b)	scalblf(a,b)

#endif

extern float acosf(float);
extern float acoshf(float);
extern float asinf(float);
extern float asinhf(float);
extern float atanf(float);
extern float atan2f(float);
extern float atanhf(float);
extern float ceilf(float);
extern float fabsf(float);
extern float floorf(float);
extern float frexpf(float, int *);
extern float hypotf(float, float);
extern float logf(float);
extern float log10f(float);
extern float log1pf(float);
extern float log2f(float);
extern float logbf(float);
extern float nanf(const char *__tagp);
extern float scalblf(float, long);
extern float scalbnf(float, int);
extern float sqrtf(float);

/* FIXME: sort out the right NaN's */
#define __sNaN       0x1.fffff0p128
#define __NaN        0x1.fffff0p128

#define __FINFINITY 1e40f

#endif
