typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

/* ACK thinks in 16bit chunks and extends 8bit when working internally. All our
   8bit indexes etc give it indigestion */

typedef unsigned short uint_fast8_t;
typedef signed short int_fast8_t;

typedef int16_t  arg_t;			/* Holds arguments */
typedef uint16_t uarg_t;		/* Holds arguments */
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint16_t uaddr_t;		/* User address */
typedef uint16_t uptr_t;		/* Userspace pointer equivalent */

#define MAXUSIZE	0xFFFF
