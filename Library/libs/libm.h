/* origin: FreeBSD /usr/src/lib/msun/src/math_private.h */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#ifndef _LIBM_H
#define _LIBM_H

#include <stdint.h>
#include <float.h>
#include <math.h>

#ifndef FORCE_EVAL
extern void __force_eval(float f);
#define FORCE_EVAL(x)	__force_eval(x)
#endif

union fshape {
	float value;
	uint32_t bits;
};


/* Check platform actually has a real double type */
#ifndef double

#ifndef NO_64BIT
union dshape {
	double value;
	uint64_t bits;
};

/* Get two 32 bit ints from a double.  */
#define EXTRACT_WORDS(hi,lo,d)                                  \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (hi) = __u.bits >> 32;                                        \
  (lo) = (uint32_t)__u.bits;                                    \
} while (0)

/* Get a 64 bit int from a double.  */
#define EXTRACT_WORD64(i,d)                                     \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (i) = __u.bits;                                               \
} while (0)

/* Get the more significant 32 bit int from a double.  */
#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (i) = __u.bits >> 32;                                         \
} while (0)

/* Get the less significant 32 bit int from a double.  */
#define GET_LOW_WORD(i,d)                                       \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (i) = (uint32_t)__u.bits;                                     \
} while (0)

/* Set a double from two 32 bit ints.  */
#define INSERT_WORDS(d,hi,lo)                                   \
do {                                                            \
  union dshape __u;                                             \
  __u.bits = ((uint64_t)(hi) << 32) | (uint32_t)(lo);           \
  (d) = __u.value;                                              \
} while (0)

/* Set a double from a 64 bit int.  */
#define INSERT_WORD64(d,i)                                      \
do {                                                            \
  union dshape __u;                                             \
  __u.bits = (i);                                               \
  (d) = __u.value;                                              \
} while (0)

/* Set the more significant 32 bits of a double from an int.  */
#define SET_HIGH_WORD(d,hi)                                     \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  __u.bits &= 0xffffffff;                                       \
  __u.bits |= (uint64_t)(hi) << 32;                             \
  (d) = __u.value;                                              \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */
#define SET_LOW_WORD(d,lo)                                      \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  __u.bits &= 0xffffffff00000000ull;                            \
  __u.bits |= (uint32_t)(lo);                                   \
  (d) = __u.value;                                              \
} while (0)

#else /* NO_64BIT */

/* Get a 32 bit int from a float.  */
union dshape {
	double value;
	uint32_t bits[2];
};

/* FIXME: double check the logic here */
#define LOBIT	0
#define HIBIT	1

/* Get two 32 bit ints from a double.  */
#define EXTRACT_WORDS(hi,lo,d)                                  \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (hi) = __u.bits[HIBIT];                                       \
  (lo) = __u.bits[LOBIT];                                       \
} while (0)

/* Get the more significant 32 bit int from a double.  */
#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (i) = __u.bits[HIBIT];                                        \
} while (0)

/* Get the less significant 32 bit int from a double.  */
#define GET_LOW_WORD(i,d)                                       \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  (i) = __u.bits[LOBIT];                                        \
} while (0)

/* Set a double from two 32 bit ints.  */
#define INSERT_WORDS(d,hi,lo)                                   \
do {                                                            \
  union dshape __u;                                             \
  __u.bits[HIBIT] = hi;                                         \
  __u.bits[LOBIT] = lo;                                         \
  (d) = __u.value;                                              \
} while (0)

/* Set the more significant 32 bits of a double from an int.  */
#define SET_HIGH_WORD(d,hi)                                     \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  __u.bits[HIBIT] = hi;                                         \
  (d) = __u.value;                                              \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */
#define SET_LOW_WORD(d,lo)                                      \
do {                                                            \
  union dshape __u;                                             \
  __u.value = (d);                                              \
  __u.bits[LOBIT] = lo;                                         \
  (d) = __u.value;                                              \
} while (0)

#endif /* NO_64BIT */

#define GET_FLOAT_WORD(i,d)                                     \
do {                                                            \
  union fshape __u;                                             \
  __u.value = (d);                                              \
  (i) = __u.bits;                                               \
} while (0)

/* Set a float from a 32 bit int.  */
#define SET_FLOAT_WORD(d,i)                                     \
do {                                                            \
  union fshape __u;                                             \
  __u.bits = (i);                                               \
  (d) = __u.value;                                              \
} while (0)

#else /* double */

/* Get a 32 bit int from a float.  */
#define GET_FLOAT_WORD(i,d)                                     \
do {                                                            \
  union fshape __u;                                             \
  __u.value = (d);                                              \
  (i) = __u.bits;                                               \
} while (0)

/* Set a float from a 32 bit int.  */
#define SET_FLOAT_WORD(d,i)                                     \
do {                                                            \
  union fshape __u;                                             \
  __u.bits = (i);                                               \
  (d) = __u.value;                                              \
} while (0)

#endif

/* fdlibm kernel functions */

int    __rem_pio2_large(double*,double*,int,int,int);

int    __rem_pio2(double,double*);
double __sin(double,double,int);
double __cos(double,double);
double __tan(double,double,int);
double __expo2(double);

int    __rem_pio2f(float,double*);
float  __sindf(double);
float  __cosdf(double);
float  __tandf(double,int);
float  __expo2f(float);

double __log1p(double);
float  __log1pf(float);

#endif
