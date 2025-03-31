#ifndef _TYPES_H
#define _TYPES_H

typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int size_t;
typedef int16_t ssize_t;

typedef unsigned char uint_fast8_t;
typedef signed char int_fast8_t;


typedef uint8_t irqflags_t;

typedef int16_t arg_t;
typedef uint16_t uarg_t;		/* Holds arguments */
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint16_t uaddr_t;		/* A user address must fit this */
typedef uint16_t uptr_t;		/* User pointer equivalent */


#define MAXUSIZE	0xFFFF


#endif
