#ifndef __STDINT_H
#define __STDINT_H

/* C types */
typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

/* C99 */
typedef int16_t intptr_t;
typedef uint16_t uintptr_t;

typedef uint16_t uint_fast8_t;
typedef int16_t int_fast8_t;
typedef uint16_t uint_fast16_t;
typedef int16_t int_fast16_t;
typedef uint32_t uint_fast32_t;
typedef int32_t int_fast32_t;
#define __SIZE_T_DEFINED
#define NO_64BIT

#define __ORDER_BIG_ENDIAN__ 4321
#define __ORDER_LITTLE_ENDIAN__ 1234

#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__

#endif
