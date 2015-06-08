/* obj.h - constants for Introl object modules */

/* Copyright (C) 1994 Bruce Evans */

#define OBJ_H

#ifndef OMAGIC
# ifdef I80386
#  define OMAGIC 0x86A3
# endif

# ifdef I8086
#  define OMAGIC 0x86A0
# endif

# ifdef MC6809
#  define OMAGIC 0x5331
# endif
#endif

#ifdef LONG_OFFSETS
# define cntooffset cnu4
# define offtocn u4cn
#else
# define cntooffset cnu2
# define offtocn u2cn
#endif

#ifdef MC6809			/* temp don't support alignment at all */
# define ld_roundup( num, boundary, type ) (num)
#else
# define ld_roundup( num, boundary, type ) \
	(((num) + ((boundary) - 1)) & (type) ~((boundary) - 1))
#endif

#define MAX_OFFSET_SIZE 4
#define NSEG 16

/* flag values |SZ|LXXXX|N|E|I|R|A|SEGM|, X not used */

#define A_MASK 0x0010		/* absolute */
#define C_MASK 0x0020		/* common (internal only) */
#define E_MASK 0x0080		/* exported */
#define I_MASK 0x0040		/* imported */
#define N_MASK 0x0100		/* entry point */
#define R_MASK 0x0020		/* relative (in text only) */
#define SEGM_MASK 0x000F	/* segment (if not absolute) */
#define SA_MASK 0x2000		/* offset is storage allocation */
#define SZ_MASK 0xC000		/* size descriptor for value */
#define SZ_SHIFT 14
