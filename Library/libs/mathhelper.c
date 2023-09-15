#include "libm.h"

#ifndef FORCE_EVAL
void __force_eval(float f)
{
}
#endif

#if defined(NO_64BIT) && !defined(double)

/* Not tested these yet */

int __isinf(double x){
    uint32_t hi;
    GET_HIGH_WORD(hi, x);
    hi &= 0x7FFFFFFFUL;
    if (hi == (0x7FFUL << 20))
        return 1;
    return 0;
}

int __isnan(double x){
    uint32_t hi;
    GET_HIGH_WORD(hi, x);
    hi &= 0x7FFFFFFFUL;
    if (hi > (0x7FFUL << 20))
        return 1;
    return 0;
}

int __isnormal(double x){
    uint32_t hi;
    GET_HIGH_WORD(hi, x);
    hi += (1UL << 20);
    hi &= 0x7FFFFFFFUL;
    if (hi >= (1UL << 21))
        return 1;
    return 0;
}

int __isfinite(double x){
    uint32_t hi;
    GET_HIGH_WORD(hi, x);
    hi &= 0x7FFFFFFFUL;
    if (hi < (0x7FFUL << 20))
        return 1;
    return 0;
}

int __signbit(double x) {
    uint32_t hi;
    GET_HIGH_WORD(hi, x);
    return hi >> 31;
}


#endif
