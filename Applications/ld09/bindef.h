
/* Only do native on Linux/i386 by default -- it's safer. */
#ifndef DETECTAOUT
#if defined(__i386__) && defined(__linux__)
#define DETECTAOUT
#else
# ifdef A_OUT_INCL
# define DETECTAOUT
# endif
#endif
#endif

#ifdef DETECTAOUT
/* Ok, I'm just gonna make it simple ... override this if you like. */
#ifndef A_OUT_INCL
#define A_OUT_INCL 	<a.out.h>
#endif

#include A_OUT_INCL

/* Try and guess type ... */
#ifndef V7_A_OUT
#ifndef BSD_A_OUT
#ifndef STANDARD_GNU_A_OUT

# ifndef C_EXT
#  define BSD_A_OUT
# endif

/* Not sure about this one ... it works here ... */
# if defined(BSD_A_OUT) && defined(N_MAGIC)
#  define STANDARD_GNU_A_OUT
# endif

#endif
#endif
#endif

/* General specs as to how it works ... */
# ifdef BSD_A_OUT
#  ifdef STANDARD_GNU_A_OUT
#   define RELOC_INFO_SIZE 8	/* unportable bitfields - bcc doesn't pack */
#  else
#   define RELOC_INFO_SIZE (sizeof (struct relocation_info))
#  endif
#  ifdef N_EXT
#   define C_EXT N_EXT
#  endif
#  define C_STAT 0
#  define n_was_name n_un.n_name
#  define n_was_numaux n_other
#  define n_was_other n_numaux
#  define n_was_sclass n_type
#  define n_was_strx n_un.n_strx
#  define n_was_type n_desc
# else /* not BSD_A_OUT */
#  define RELOC_INFO_SIZE (sizeof (struct reloc))
#  define n_was_name n_name
#  define n_was_numaux n_numaux
#  define n_was_other n_other
#  define n_was_sclass n_sclass
#  define n_was_strx n_value
#  define n_was_type n_type
# endif /* BSD_A_OUT */

/* And finally make sure it worked */
#if defined(A_MINHDR) || defined(BSD_A_OUT)
#if defined(C_EXT) && defined(C_STAT) && !defined(SCNHSZ)

#define AOUT_DETECTED	1

#endif
#endif

#endif /* NO_AOUT */
