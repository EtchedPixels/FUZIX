/*
 *	Fairly boringly generic stdarg. Everythign is word aligned.
 */

#ifndef _STDARG_H
#define	_STDARG_H

typedef char* va_list;

#define __typesize(__type)	((sizeof(__type)+sizeof(int)-1) & ~(sizeof(int) -1))

#define va_start(__ap, __last)	(__ap = (va_list)&__last + __typesize(__last))
#define va_end(__ap)

#define va_arg(__ap, __type)	(*((__type *)(void *)((__ap += __typesize(__type)) - __typesize(__type))))

#endif
