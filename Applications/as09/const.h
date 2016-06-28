
/* Speed and space hacks for BCC */
#ifdef __AS386_16__
#define LOW_BYTE	0	/* must be changed for big-endian */
#else

#define S_ALIGNMENT	sizeof(long)
#endif

/* const.h - constants for assembler */

/* major switches */

#define MC6809			/* generate 6809 code */
#ifndef MC6809
#define I80386			/* generate 80386 code */
#endif
#define MNSIZE			/* allow byte size in mnemonic, e.g. "movb" */

/* defaults */

#define DIRCHAR		'/'	/* character separating filename from dir */
#define INBUFSIZE	512
#define SOS_EOLSTR	"\012"

/* defaults modified by switches */

#ifdef __AS386_16__
# undef INBUFSIZE
# define INBUFSIZE	512
# define STAKSIZ	512	/* table grows up to stack less this */
#endif

/* booleans */

#define FALSE		0
#define TRUE		1

/* ASCII constants */

#define ETB		23

/* C tricks */

#define EXTERN		extern
#define NUL_PTR		((void*)0)

/* O/S constants */

#define CREAT_PERMS	0666
#define EOF		(-1)
#define STDIN		0
#define STDOUT		1

enum
{
/* Register codes (internal to assembler). */
#ifdef I80386

/* Index regs must be first. */
    BPREG,
    BXREG,
    DIREG,
    SIREG,
#define MAX16BITINDREG	SIREG

    EAXREG,
    EBPREG,
    EBXREG,
    ECXREG,
    EDIREG,
    EDXREG,
    ESIREG,
    ESPREG,
#define MAXINDREG	ESPREG

    AXREG,
    CXREG,
    DXREG,
    SPREG,

    AHREG,
    ALREG,
    BHREG,
    BLREG,
    CHREG,
    CLREG,
    DHREG,
    DLREG,

    CSREG,
    DSREG,
    ESREG,
    FSREG,
    GSREG,
    SSREG,

    CR0REG,
    CR2REG,
    CR3REG,
    DR0REG,
    DR1REG,
    DR2REG,
    DR3REG,
    DR6REG,
    DR7REG,
    TR3REG,
    TR4REG,
    TR5REG,
    TR6REG,
    TR7REG,

    ST0REG,
    ST1REG,
    ST2REG,
    ST3REG,
    ST4REG,
    ST5REG,
    ST6REG,
    ST7REG,
#endif /* I80386 */

#ifdef MC6809

/* Index regs must be first, then PC. */
    SREG,
    UREG,
    XREG,
    YREG,
#define MAXINDREG	YREG

    PCREG,
    AREG,
    BREG,
    CCREG,
    DPREG,
    DREG,
#endif /* MC6809 */

    NOREG
};

#ifdef I80386
enum
{
/* Type and size keywords. */
    BYTEOP,
    DWORDOP,
    FWORDOP,
    FAROP,
    PTROP,
    PWORDOP,
    QWORDOP,
    TBYTEOP,
    WORDOP
};
#endif /* I80386 */

/* special chars */

#define EOL		0
#define MACROCHAR	'?'

enum
{
/* Symbol codes. */

/* The first 2 must be from chars in identifiers. */
    IDENT,
    INTCONST,

/* The next few are best for other possibly-multi-char tokens. */
    ADDOP,			/* also ++ */
    BINCONST,
    CHARCONST,
    GREATERTHAN,		/* also >> and context-sensitive */
    HEXCONST,
    LESSTHAN,			/* also << and context-sensitive */
    SUBOP,			/* also -- */
    WHITESPACE,

    ANDOP,
    COMMA,
    EOLSYM,
    EQOP,
    IMMEDIATE,
    INDIRECT,
    LBRACKET,
    LPAREN,
    MACROARG,
    NOTOP,
    OROP,
    OTHERSYM,
    POSTINCOP,
    PREDECOP,
    RBRACKET,
    RPAREN,
    SLASH,			/* context-sensitive */
    SLOP,
    SROP,
    STAR,			/* context-sensitive */
    STRINGCONST,
    COLON
};

/* symbol table entry */

				/* type entry contains following flags */
#define ENTBIT		(1<<0)	/* entry point (= OBJ_N_MASK) */
#define COMMBIT		(1<<1)	/* common */
#define LABIT		(1<<2)	/* label (a PC location or defined by EQU) */
#define MNREGBIT	(1<<3)	/* mnemonic for op or pseudo-op, or register */
#define MACBIT		(1<<4)	/* macro */
#define REDBIT		(1<<5)	/* redefined (if with LABIT or VARBIT), to do
				 * with SA_MASK (if with COMMBIT), otherwise
				 * means globl */
#define VARBIT		(1<<6)	/* variable (i.e. something defined by SET) */
#define EXPBIT		(1<<7)	/* exported (= OBJ_E_MASK) */

				/* data entry contains following flags, valid */
				/* for expressions as well as syms */
#define PAGE1		(1<<0)	/* page 1 machine op = MNREGBIT | PAGE1 */
#define PAGE2		(1<<1)	/* page 2 machine op = MNREGBIT | PAGE2 */
#define REGBIT		(1<<2)	/* register = MNREGBIT | REGBIT */
#define SIZEBIT		(1<<3)	/* sizing mnemonic = MNREGBIT | SIZEBIT */
#define SEGM		0x0F	/* 1st 4 bits reused for segment if !MNREGBIT */
#define RELBIT		(1<<4)	/* relative (= OBJ_A_MASK) */
#define FORBIT		(1<<5)	/* forward referenced */
#define IMPBIT		(1<<6)	/* imported (= OBJ_I_MASK) */
#define UNDBIT		(1<<7)	/* undefined */

/* object code format (Introl) */

#define OBJ_SEGSZ_TWO	0x02	/* size 2 code for segment size descriptor */

#define OBJ_MAX_ABS_LEN	64	/* max length of chunk of absolute code */

#define OBJ_ABS		0x40	/* absolute code command */
#define OBJ_OFFSET_REL	0x80	/* offset relocation command */
#define OBJ_SET_SEG	0x20	/* set segment command */
#define OBJ_SKIP_1	0x11	/* skip with 1 byte count */
#define OBJ_SKIP_2	0x12	/* skip with 2 byte count */
#define OBJ_SKIP_4	0x13	/* skip with 4 byte count */
#define OBJ_SYMBOL_REL	0xC0	/* symbol relocation command */

#define OBJ_A_MASK	0x10	/* absolute bit(symbols) */
#if OBJ_A_MASK - RELBIT		/* must match internal format (~byte 1 -> 0) */
oops - RELBIT misplaced
#endif
#define OBJ_E_MASK	0x80	/* exported bit (symbols) */
#if OBJ_E_MASK - EXPBIT		/* must match internal format (byte 0 -> 0) */
oops - EXPBIT misplaced
#endif
#define OBJ_I_MASK	0x40	/* imported bit (symbols) */
#if OBJ_I_MASK - IMPBIT		/* must match internal format (byte 1 -> 0) */
oops - IMPBIT misplaced
#endif
#define OBJ_N_MASK	0x01	/* entry bit (symbols) */
#if OBJ_N_MASK - ENTBIT		/* must match internal format (byte 0 -> 1) */
oops - ENTBIT misplaced
#endif
#define OBJ_SA_MASK	0x20	/* size allocation bit (symbols) */
#define OBJ_SZ_ONE	0x40	/* size one code for symbol value */
#define OBJ_SZ_TWO	0x80	/* size two code for symbol value */
#define OBJ_SZ_FOUR	0xC0	/* size four code for symbol value */

#define OBJ_R_MASK	0x20	/* PC-rel bit (off & sym reloc commands) */
#define OBJ_SEGM_MASK	0x0F	/* segment mask (symbols, off reloc command) */

#define OBJ_OF_MASK	0x03	/* offset size code for symbol reloc */
#define OBJ_S_MASK	0x04	/* symbol number size code for symbol reloc */

#define SYMLIS_NAMELEN	26
#define SYMLIS_LEN	(sizeof (struct sym_listing_s))

#define FILNAMLEN	64	/* max length of a file name */
#define LINLEN		256	/* max length of input line */
#define LINUM_LEN	5	/* length of formatted line number */

#define SPTSIZ		1024	/* number of symbol ptrs */
				/* pseudo-op flags */
#define POPHI		1	/* set to print hi byte of adr */
#define POPLO		2	/* to print lo byte of ADR */
#define POPLC		4	/* to print LC */
#define POPLONG		8	/* to print high word of ADR */
#define MAXBLOCK	8	/* max nesting level of BLOCK stack */
#define MAXGET		8	/* max nesting level of GET stack */
#define MAXIF		8	/* max nesting level of IF stack */
#define MACPSIZ		(128 / sizeof (struct schain_s))
				/* size of macro param buffer */
#define MAXMAC		8	/* max nesting level of macro stack */
#define NLOC		16	/* number of location counters */
#ifdef I80386
#define NO_SIB		0340	/* illegal sib (3 with 4) to mean no sib */
#endif

/* special segments */

#define TEXTLOC		0
#define DATALOC		1
#define BSSLOC		2
#define DPLOC		3

#include "errors.h"

