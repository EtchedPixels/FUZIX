#ifndef __FEATURES_H
#define __FEATURES_H

#ifndef __STDC__
#ifdef HI_TECH_C
#define __STDC__	1
#endif
#endif

#ifdef __STDC__

#ifndef VOID
#define VOID	void
#endif
#ifndef __P
#define __P(a)	a
#endif
#define __const const

#else	/* K&R */

#ifndef VOID
#define VOID
#endif
#ifndef __P
#define __P(a)	()
#endif
#define __const
#define volatile
#define void	char

#endif	/* __STDC__ */

#include <sys/cdefs.h>

#endif

