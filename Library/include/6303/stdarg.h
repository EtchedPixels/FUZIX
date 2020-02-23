/*
 *	Needs reviewing
 */
#ifndef _STDARG_H
#define	_STDARG_H

#include <stdint.h>

typedef uint8_t *va_list;

#define va_start(__ap, __last)	((__ap) = (va_list)(&(__ap)) + sizeof(__last))
#define va_end(__ap)

#define va_arg(__ap, __type)	(*((__type *)(void *)((__ap += sizeof(__type)) - sizeof(__type))))

#endif
