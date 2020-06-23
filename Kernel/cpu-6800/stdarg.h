/*
 *	This is a bit strange because our argments are left to right on
 *	the stack.
 */
#ifndef _STDARG_H
#define	_STDARG_H

typedef unsigned char *va_list;

#define __typesize(__type)	(sizeof(__type) == 1 ? sizeof(int) : sizeof(__type))

#define va_start(__ap, __last)	((__ap) = (va_list)(&(__last)))
#define va_end(__ap)

#define va_arg(__ap, __type)	(*((__type *)(void *)(__ap -= __typesize(__type))))

#endif
