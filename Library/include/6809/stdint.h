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

#define __SIZE_T_DEFINED
#define NO_64BIT

#endif
