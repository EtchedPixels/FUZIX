/*
 * Z-80 assembler.
 * Header file, used by all
 * parts of the assembler.
 */
#include	<stdio.h>
#include	<string.h>
#include 	<stdlib.h>
#include	<stdint.h>
#include	<ctype.h>
#include	<setjmp.h>

/*
 * Table sizes, etc.
 */
#define	NCPS	16			/* # of characters in symbol */
#define	NHASH	64			/* # of hash buckets */
#define	HMASK	077			/* Mask for above */
#define	NFNAME	32			/* # of characters in filename */
#define	NERR	10			/* Size of error buffer */
#define	NCODE	128			/* # of characters in code buffer */
#define	NINPUT	128			/* # of characters in input line */
#define	NLPP	60			/* # of lines on a page */
#define	XXXX	0			/* Unused value */

/*
 * Exit codes.
 */
#define	GOOD	0
#define	BAD	1

#ifdef TARGET_Z80

typedef	uint16_t	VALUE;		/* For symbol values */

#define NSEGMENT 4			/* # of segments */

#define ARCH OA_8080
#define ARCH_FLAGS 0
#define ARCH_CPUFLAGS OA_8080_Z80

/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x007F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define	TNOP	0x0F00			/* nop */
#define	TRST	0x1000			/* restarts */
#define	TREL	0x1100			/* djnz, jr */
#define	TRET	0x1200			/* ret */
#define	TJMP	0x1300			/* call, jp */
#define	TPUSH	0x1400			/* push, pop */
#define	TIM	0x1500			/* im */
#define	TIO	0x1600			/* in, out */
#define	TBIT	0x1700			/* set, res, bit */
#define	TSHR	0x1800			/* sl, sr et al */
#define	TINC	0x1900			/* inc, dec */
#define	TEX	0x1A00			/* ex */
#define	TADD	0x1B00			/* add, adc, sbc */
#define	TLD	0x1C00			/* ld */
#define	TCC	0x1D00			/* condition code */
#define	TSUB	0x1E00			/* sub et al */
#define TNOP180	0x2100			/* Z180 immediate */
#define TTST180	0x2200			/* TST m/g/(hl) */
#define TIMMED8	0x2300			/* TSTIO m */
#define	TMUL	0x2400			/* MUL */
#define TIO180	0x2500			/* IN0/OUT0 */

/*
 * Registers.
 */
#define	B	0
#define	C	1
#define	D	2
#define	E	3
#define	H	4
#define	L	5
#define	M	6
#define	A	7
#define	IX	8
#define	IY	9

#define	BC	0
#define	DE	1
#define	HL	2
#define	SP	3
#define	AF	4
#define	AFPRIME	5

#define	I	0
#define	R	1

/*
 * Condition codes.
 */
#define	CNZ	0
#define	CZ	1
#define	CNC	2
#define	CC	3
#define	CPO	4
#define	CPE	5
#define	CP	6
#define	CM	7

/*
 *	Error message numbers: FIXME - sort general first
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define CONDCODE_ONLY	18
#define INVALID_REG	19
#define ADDR_REQUIRED	20
#define INVALID_ID	21
#define REG_MUST_BE_C	22
#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define REQUIRE_Z180	28
#define	SEGMENT_CLASH	29
#define UNKNOWN_SYMBOL	30


#elif TARGET_6502

typedef	uint16_t	VALUE;		/* For symbol values */

#define NSEGMENT 5			/* # of segments */

#define ARCH OA_6502
#define ARCH_FLAGS 0
#define ARCH_CPUFLAGS OA_6502_BCD	/* For now until CPU type properly settable */


/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x000F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */
#define TMADDR	0x00F0			/* Addressing mode bits */

#define	TZP	0x0010			/* 0000 is TUSER */
#define TACCUM	0x0020
#define TZPX	0x0030
#define TZPY	0x0040
#define TABSX	0x0050
#define TABSY	0x0060
#define TZPX_IND	0x0070
#define TZPY_IND	0x0080
#define TZP_IND	0x0090


#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define TCC	0x0F00
/* CPU specific codes */
#define	TCLASS0	0x1000			/* xxxyyy00 instructions */
#define TCLASS1	0x1100			/* xxxyyy01 instructions */
#define TCLASS2	0x1200			/* xxxyyy10 instructions */
#define TCLASS2Y 0x1300			/* ditto but taking Y */
#define TJMP	0x1400			/* JMP */
#define TREL8	0x1500			/* Bcc */
#define TIMPL	0x1600			/* Implicit */
#define TBRK	0x1700			/* BRK */
#define TJSR	0x1800			/* JSR */

/*
 * Registers.
 */
#define	A	0
#define	X	1
#define	Y	2

/*
 *	Error message numbers
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define CONDCODE_ONLY	18
#define INVALID_REG	19
#define ADDR_REQUIRED	20
#define INVALID_ID	21
#define BADMODE		22
#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define	SEGMENT_CLASH	28

#elif TARGET_DGNOVA

#define TARGET_WORD_MACHINE

/* 16 bit machine but we need to track in 32bits to allow for the fact we
   can be dealing with 2^16 words */
typedef	uint32_t	VALUE;		/* For symbol values */

#define SEGMENT_LIMIT	0x10000		/* bytes */

#define NSEGMENT 5			/* # of segments */

#define ARCH OA_DGNOVA
#define ARCH_FLAGS OF_WORDMACHINE
#define ARCH_CPUFLAGS 0


/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x000F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define TCC	0x0F00
/* CPU specific codes */
#define TCPUOPT	0x1100
#define TMEMORY	0x1200
#define TALU	0x1300
#define TIO	0x1400
#define TDEV	0x1500
#define TAC	0x1600
#define TIMPL	0x1700
#define TBYTE	0x1800
#define TTRAP	0x1900

#define TPCREL	0x0010

/*
 *	Error message numbers
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define CONDCODE_ONLY	18
#define INVALID_REG	19
#define ADDR_REQUIRED	20
#define INVALID_ID	21
#define BADMODE		22
#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define BAD_ACCUMULATOR	28
#define NEED_ZPABS	29
#define BADDEVICE	30
#define BAD_PCREL	BRA_RANGE
#define	SEGMENT_CLASH	31

#elif TARGET_6809

typedef	uint16_t	VALUE;		/* For symbol values */

#define NSEGMENT 4			/* # of segments */

#define ARCH OA_6809
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS 0

#define TARGET_BIGENDIAN
#define TARGET_USES_SQUARE

/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x007F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
/* CPU specific codes */
#define THI	0x1100			/* all modes opcodes 0x80+ */
#define THIW	0x1200			/* ditto word mode */
#define THINOIMM 0x1300			/* ditto byte no immediate */
#define THIWNOIMM 0x1400		/* ditto word no immediate */
#define TIMM8	0x1500			/* 8bit immediate only */
#define TEXG	0x1600			/* registers as 8bit immediate */
#define TPUSH	0x1700			/* register mask as 8bit immediate */
#define TIMP	0x1800			/* implied by instruction */
#define TLO	0x1900			/* low instructions with some modes */
#define TLEA	0x1A00			/* lea */
#define TBR	0x1B00			/* branch */
#define TLBR	0x1C00			/* long branch */

#define TAIMM	0x0000			/* #x :immediate mode 8bit */
#define TADIR	0x0010			/* dp:x */
#define TAIND	0x0020			/* Indexed postbyte forms */
#define TAEXT	0x0030			/* xx : extended addressing */

/*
 * Registers.
 */
#define	A	0		/* re-arrange to match bit patterns */
#define	B	1
#define	DP	2
#define	CC	3
#define	D	4
#define	U	5
#define	X	6
#define	Y	7
#define	S	8
#define PCR	9

/*
 *	Error message numbers: FIXME - sort general first
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define CONDCODE_ONLY	18
#define INVALID_REG	19
#define ADDR_REQUIRED	20
#define INVALID_ID	21
#define REG_MUST_BE_C	22
#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define REQUIRE_Z180	28
#define	SEGMENT_CLASH	29

#elif TARGET_6303

typedef	uint16_t	VALUE;		/* For symbol values */

#define NSEGMENT 5			/* # of segments */

#define ARCH OA_6800
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS OA_6800_6303		/* For now until CPU type properly settable */


/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x000F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */
#define TMADDR	0x00F0			/* Addressing mode bits */

#define TDIRECT	0x0010			/* Direct page */
#define TINDEX	0x0020			/* Indexed */
#define TIMMED	0x0030			/* Immediate */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define TCC	0x0F00
/* CPU specific codes */
#define TIMPL	0x1000			/* Implicit */
/* 0x2x and 0x8D */
#define TREL8	0x1100			/* 8bit relative from . + 2 */
/* 0x6X,0x7X */
#define TXE	0x1200			/* Extended or indexed */
/* 0x8x-0xFx */
#define TDIXE	0x1300			/* All forms */
/* Stores in that range */
#define TDXE	0x1400			/* No immediate (eg store) */
/* 0x8x-0xFx */
#define T16DIXE	0x1500			/* All forms double immediate*/
/* Stores in that range */
#define T16DXE	0x1600			/* Double immediate no store */
/* 6303 specials */
#define TIMPL6303	0x1700		/* 6303 implicit */
#define TIDX6303	0x1800		/* AIM etc */
#define TIDXB6303	0x1900		/* BIT form of the above */

/*
 * Registers.
 */
#define	A	0
#define B	1
#define	D	2
#define	X	3

/*
 *	Error message numbers
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define INDX_RANGE	18
#define ADDR_REQUIRED	19
#define INVALID_ID	20
#define BADMODE		21
#define DIVIDE_BY_ZERO	22
#define CONSTANT_RANGE  23
#define DATA_IN_BSS	24
#define SEGMENT_OVERFLOW 25
#define DATA_IN_ZP	26
#define	SEGMENT_CLASH	27

#elif TARGET_Z8

typedef	uint16_t	VALUE;		/* For symbol values */

#define NSEGMENT 5			/* # of segments */

#define ARCH OA_Z8
#define ARCH_FLAGS 0
#define ARCH_CPUFLAGS 0			/* For now until CPU type properly settable */


/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x000F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */
#define TMADDR	0x00F0			/* Addressing mode bits */

#define TRS	0x0010			/* Register, short form */
#define TREG	0x0020			/* Register */
#define TIMMED	0x0030			/* Immediate */
#define TSIND	0x0040			/* Register short indirect */
#define TIND	0x0050			/* Indrect */
#define TRR	0x0060			/* Register pair */
#define TRRIND	0x0070			/* Register pair indirect */
#define TINDEX	0x0080			/* value is followed by (xx) */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define TCC	0x0F00
/* CPU specific codes */
#define TIMPL	0x1000			/* Implicit */
#define TOP4BIT	0x1100			/* 4bit + mode */
#define TRIR	0x1200			/* R or IR mode */
#define TRRIR	0x1300			/* RR or IR mode */
#define TCRA	0x1400			/* JR CC, rel */
#define TJMP	0x1500			/* JP cc,DA + JP cc,IRR */
#define TIMM8	0x1600			/* Only used for SRP */
#define TIRRDA	0x1700			/* For CALL */
#define TRA	0x1800			/* Relative address */
#define TLDC	0x1900			/* LDC (r,Irr  / Irr, Ir ) */
#define TLDCI	0x1A00			/* LDCI (Ir,Irr / Irr, Ir) */
#define TLOAD	0x1B00			/* 12 forms.. */

/*
 *	Error message numbers
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define RSHORT_RANGE	18
#define ADDR_REQUIRED	19
#define INVALID_ID	20
#define INVALID_FORM	21
#define DIVIDE_BY_ZERO	22
#define CONSTANT_RANGE  23
#define DATA_IN_BSS	24
#define SEGMENT_OVERFLOW 25
#define DATA_IN_ZP	26
#define	SEGMENT_CLASH	27
#define ODD_REGISTER	28

#elif TARGET_1802

typedef	uint16_t	VALUE;		/* For symbol values */

#define NSEGMENT 5			/* # of segments */

#define ARCH OA_1802
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS 0			/* For now until CPU type properly settable */


/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x000F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */
#define TMADDR	0x00F0			/* Addressing mode bits */

#define TIMMED	0x0010			/* Immediate */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define TCC	0x0F00
/* CPU specific codes */
#define TIMPL	0x1000			/* Implicit */
#define TIMM8	0x1100			/* Immediate 8bit */
#define TNOP	0x1200			/* NOP */
#define TSKIP	0x1300			/* LSKIP */
#define TREG	0x1400			/* 0-15 */
#define TREGNZ	0x1500			/* 1-15 */
#define TADDR16	0x1600			/* Long address */
#define TIOPORT	0x1700			/* I/O port num (1-7) */
#define TREL	0x1800			/* Relative address (sort of)*/

/*
 *	Error message numbers
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define INVALID_REG	18
#define ADDR_REQUIRED	19
#define INVALID_ID	20
#define INVALID_IO	21
#define DIVIDE_BY_ZERO	22
#define CONSTANT_RANGE  23
#define DATA_IN_BSS	24
#define SEGMENT_OVERFLOW 25
#define DATA_IN_ZP	26
#define	SEGMENT_CLASH	27
#define NOT_REG0	28

#else
#error "Unknown target"
#endif

/*
 * Segments
 */
#define UNKNOWN		15
#define ABSOLUTE	0
#define CODE		1
#define DATA		2
#define BSS		3

/*
 * Expression priority
 */

#define	LOPRI	0
#define	ADDPRI	1
#define	MULPRI	2
#define	HIPRI	3

#ifndef SEGMENT_LIMIT
#define SEGMENT_LIMIT	0
#endif

/*
 * Address description.
 */
typedef	struct	ADDR	{
	uint16_t a_type;			/* Type */
	VALUE	a_value;		/* Index offset, etc */
	uint8_t	a_segment;		/* Segment relative to */
	uint8_t a_flags;		/* To track high */
#define A_HIGH		1
#define A_LOW		2
	struct SYM *a_sym;		/* Symbol tied to this address */
					/* NULL indicates simple own segment */
}	ADDR;

/*
 * Symbol.
 */
typedef	struct	SYM	{
	struct	SYM *s_fp;		/* Link in hash */
	char	s_id[NCPS];		/* Name */
	int	s_type;			/* Type */
	VALUE	s_value;		/* Value */
	uint16_t s_number;		/* Symbol number 1..n, also usable for
					   tokens as extra data */
	int	s_segment;		/* Segment this symbol is relative to */
}	SYM;

/*
 * External variables.
 */
extern	char	*cp;
extern	char	*ip;
extern	char	ib[];
extern	FILE	*ifp;
extern	FILE	*ofp;
extern	FILE	*lfp;
extern	int	line;
extern	int	lmode;
extern	VALUE	laddr;
extern	SYM	sym[];
extern	int	pass;
extern	SYM	*phash[];
extern	SYM	*uhash[];
extern	int	lflag;
extern	jmp_buf	env;
extern	VALUE	dot[NSEGMENT];
extern  int	segment;
extern	int	debug_write;
extern	char	*fname;
extern	int	noobj;
extern	int	cpu_flags;

extern void asmline(void);
extern void comma(void);
extern void istuser(ADDR *);
extern int symhash(char *);
extern void err(char, uint8_t);
extern void uerr(char *);
extern void aerr(uint8_t);
extern void qerr(uint8_t);
extern void storerror(int);
extern void getid(char *, int);
extern SYM *lookup(char *, SYM *[], int);
extern int symeq(char *, char *);
extern void symcopy(char *, char *);
extern int get(void);
extern int getnb(void);
extern void unget(int);
extern void getaddr(ADDR *);
extern void expr1(ADDR *, int, int);
extern void expr2(ADDR *);
extern void expr3(ADDR *, int);
extern void isokaors(ADDR *, int);
extern void outpass(void);
extern void outabsolute(int);
extern void outsegment(int);
extern void outaw(uint16_t);
extern void outab(uint8_t);
extern void outabchk(uint16_t);
extern void outraw(ADDR *);
extern void outrab(ADDR *);
extern void outrabrel(ADDR *);
extern void outeof(void);
extern void outbyte(uint8_t);
extern void outflush(void);
extern void syminit(void);

extern char *etext[];

#include "obj.h"
