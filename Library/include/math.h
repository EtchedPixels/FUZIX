/*
 */

#ifndef _MATH_H
#define _MATH_H

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
extern double pow(double, double);
extern double exp(double);

extern double eval_poly(double, double *, int);
#endif
