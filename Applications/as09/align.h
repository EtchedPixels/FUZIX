/* align.h - memory alignment requirements for linker */

/* Copyright (C) 1994 Bruce Evans */

#ifndef S_ALIGNMENT
# define align(x)
#else

#if defined(__STDC__) && defined(_POSIX_SOURCE)
# define align(x) ((x)=(void *)	\
		   (((ssize_t)(x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1)))
#else
# define align(x) ((x)=(void *)	\
		   ((char *)(x) + ((S_ALIGNMENT-(char)(x)) & (S_ALIGNMENT-1))))
#endif
#endif







