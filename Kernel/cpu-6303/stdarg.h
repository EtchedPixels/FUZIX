/*
 *	This is a bit strange because our argments are left to right on
 *	the stack.
 */
#ifndef _STDARG_H
#define	_STDARG_H

typedef unsigned char *va_list;

#define va_start(__ap, __last)	((__ap) = (va_list)(&(__ap)))
#define va_end(__ap)

#define va_arg(__ap, __type)	(*((__type *)(void *)(__ap -= sizeof(__type))))

#endif
