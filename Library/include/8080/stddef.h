#ifndef _STDDEF_H
#define	_STDDEF_H

#include <stdint.h>

#define	NULL ((void *)0)

#define	offsetof(type, ident)	((size_t) &((type *)0)->ident)

#endif
