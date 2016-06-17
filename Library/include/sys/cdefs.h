#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H
#ifndef __FEATURES_H
#include <features.h>
#endif

#ifdef __STDC__
#define __CONCAT(x,y)	x ## y
#define __STRING(x)	#x
#else
#define __CONCAT(x,y)	x/**/y
#define __STRING(x)	#x
#endif	/* __STDC__ */

/* This is not a typedef so `const __ptr_t' does the right thing */
#define __ptr_t void *

#endif

