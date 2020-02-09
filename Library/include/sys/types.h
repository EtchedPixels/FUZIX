#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stddef.h>

#if defined (__STDC__)
#include <stdint.h>
#else
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
#endif

/* USER! basic data types */
/* ! uchar & uint is counterparts and must be declared simultaneously */

#ifndef uchar_is_defined
#define uchar_is_defined
typedef unsigned char uchar;
typedef unsigned int uint;
#endif

#if !defined(__SIZE_T_DEFINED) && !defined(_SIZE_T_DEFINED)
#define __SIZE_T_DEFINED
typedef uint16_t size_t;
#endif

#if !defined(__SSIZE_T_DEFINED) && !defined(_SSIZE_T_DEFINED)
#define __SSIZE_T_DEFINED
typedef int16_t ssize_t;
#endif


/* Unix historic */
typedef unsigned char uchar_t;
typedef unsigned long ulong_t;

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef char * caddr_t;
typedef long daddr_t;
typedef long key_t;

/* Unix types for properties */
typedef uint16_t mode_t;
typedef int32_t off_t;
typedef uint16_t dev_t;
typedef uint16_t gid_t;
typedef uint16_t uid_t;
typedef uint16_t nlink_t;
typedef int16_t pid_t;
typedef uint16_t ino_t;

typedef uint16_t fsblkcnt_t;
typedef uint16_t fsfilcnt_t;

#if defined(NO_64BIT)
typedef uint32_t time_t;
/* For kernel struct alignment */
typedef struct {
  uint32_t time;
  uint32_t pad;
} __ktime_t;
#else
typedef int64_t time_t;
typedef struct {
  uint64_t time;
} __ktime_t;
#endif
typedef int32_t clock_t;
typedef uint32_t useconds_t;

#define makedev(a,b)	(((a) << 8) | (b))
#define major(x)	((x) >> 8)
#define minor(x)	((x) & 0xFF)

#endif
