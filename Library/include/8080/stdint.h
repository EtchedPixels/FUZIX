#ifndef _STDINT_H
#define _STDINT_H

typedef signed char     int8_t;
typedef unsigned char   uint8_t;
#define INT8_MAX        127
#define INT8_MIN        (-128)
#define UINT8_MAX       255

typedef signed short    int16_t;
typedef unsigned short  uint16_t;
#define INT16_MAX       32767
#define INT16_MIN       (-32768)
#define UINT16_MAX      65535

typedef signed long     int32_t;
typedef unsigned long   uint32_t;
#define INT32_MAX       2147483647
#define INT32_MIN       (-2147483648)
#define UINT32_MAX      4294967295

typedef int16_t         intptr_t;
typedef uint16_t        uintptr_t;
typedef int16_t         ptrdiff_t;
#define INTPTR_MAX      32767
#define INTPTR_MIN      (-32768)
#define UINTPTR_MAX     65535

typedef uint8_t uint_fast8_t;
typedef int8_t int_fast8_t;
typedef uint16_t uint_fast16_t;
typedef int16_t int_fast16_t;
typedef uint32_t uint_fast32_t;
typedef int32_t int_fast32_t;

#define PTRDIFF_MAX     INTPTR_MAX
#define PTRDIFF_MIN     INTPTR_MIN
#define SIZE_MAX        UINTPTR_MAX

#define NO_64BIT

/* Ugly hack for the maths lib for the moment */
#define double float

#endif
