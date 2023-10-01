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

#include	"obj.h"

/*
 * Table sizes, etc.
 */
#define	NCPS	NAMELEN			/* # of characters in symbol */
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
#define TSETCPU 0x2600			/* .setcpu */

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
#define REG_MUST_BE_C	22
#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define REQUIRE_Z180	28
#define	SEGMENT_CLASH	29
#define UNKNOWN_SYMBOL	30
#define TOO_MANY_JR	31

#elif TARGET_8085

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_8080
#define ARCH_FLAGS 0
#define ARCH_CPUFLAGS 0

/* We generate intentionally wrapping 16bit maths for relocations */
#define TARGET_RELOC_OVERFLOW_OK

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
#define	TSETCPU	0x0F00			/* setcpu */
#define	TIMPL	0x1000			/* implicit 8080 */
#define	TIMPL85	0x1100			/* implicit 8085 */
#define	TI8	0x1200			/* 8bit immediate */
#define	TI8_85	0x1300			/* 8bit immediate 8085 */
#define	TI16	0x1400			/* 16bit immediate */
#define	TI16_85	0x1500			/* 16bit immediate 8085 */
#define	TREG8	0x1600			/* 8bit register */
#define	TREG16	0x1700			/* 16bit register (sp allowed not psw )*/
#define	TREG16_P 0x1800			/* 16bit push/pop (psw allowed not sp) */
#define	TMOV	0x1900			/* mov instruction */
#define TRST    0x1A00			/* rst instruction */
#define TCC	0x1B00			/* not used but needed for core code */
#define TREG8_I8 0x1C00			/* reg,constant - mvi etc */
#define TREG16_I16 0x1D00			/* reg,constant - mvi etc */
#define	TREG8H	0x1E00			/* 8bit register bits 5-3 (inc/dec) */
#define TREG16BD 0x1F00			/* stax/ldax */

/*
 * Registers.
 */
#define	B	0			/* 8bit matching encoding for 8 */
#define	C	1
#define	D	2
#define	E	3
#define	H	4
#define	L	5
#define	M	6
#define	A	7

#define	SP	8			/* 16bit cases */
#define	PSW	9

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


#define INVALID_REG	19
#define ADDR_REQUIRED	20
#define INVALID_ID	21

#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define REQUIRE_8085	28
#define	SEGMENT_CLASH	29
#define UNKNOWN_SYMBOL	30


#elif TARGET_6502

typedef	uint16_t	VALUE;		/* For symbol values */

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

#define	TZP		0x0010		/* 0000 is TUSER */
#define TACCUM		0x0020
#define TZPX		0x0030
#define TZPY		0x0040
#define TABSX		0x0050
#define TABSY		0x0060
#define TZPX_IND	0x0070
#define TZPY_IND	0x0080
#define TZP_IND		0x0090
#define TZP_INDL	0x00A0
#define TALX_IND	0x00B0
#define TZPYL_IND	0x00C0
#define TSR		0x00D0
#define TSRY_IND	0x00E0
#define TABSL		0x00F0

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
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
#define TBRA16	0x1900			/* Jcc asm magic */
#define TI	0x1A00			/* Set index size */
#define TA	0x1B00			/* Set accum size */
#define TCPU	0x1C00			/* Set CPU type */
#define TIMPLC	0x1D00			/* 65C02 implicit */
#define TIMPL16	0x1E00			/* 65C816 implicit */
#define TCLASS2A 0x1F00			/* INC/DEC A */
#define TIMM16	0x2000			/* 16bit immediate 65C816 */
#define TSTZ	0x2100			/* Store zero 65C02 */
#define TABDP	0x2200			/* Ops taking abs,dp only */
#define TMVN	0x2300			/* MVN/MVP 65C816 */
#define TPEI	0x2400			/* PEI 65C816 */
#define TREP	0x2500			/* REP 65C816 */
#define TJML	0x2600			/* JML 65C816 */
#define TLONG	0x2700			/* Abslute 24bit long 65C816 */
#define TREL8C	0x2800			/* 8bit rel, 65C02 */
#define TREL16	0x2900			/* 16bit rel, 65C816 */
#define TCLASS0X 0x2A00			/* bit is slightly odd */

/*
 * Registers.
 */
#define	A	0
#define	X	1
#define	Y	2

/*
 *	 CPU types
 */

#define CPU_6502		0
#define CPU_65C02		1
#define CPU_65C816		2

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
#define RANGE		29
#define BADCPU		30
#define TOO_MANY_BRA	31

#elif TARGET_DGNOVA

#define TARGET_WORD_MACHINE

/* 16 bit machine but we need to track in 32bits to allow for the fact we
   can be dealing with 2^16 words */
typedef	uint32_t	VALUE;		/* For symbol values */

#define SEGMENT_LIMIT	0x10000		/* bytes */

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

#define ARCH OA_6800
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS OA_6800_6303		/* For now until CPU type properly settable */

#define TARGET_BIGENDIAN

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
/* Assembler extras for resolving branch ranges */
#define TBRA16		0x1A00		/* Jcc asm magic */
#define TSETCPU		0x1B00		/* .setcpu */
/* 6803/303 specifics */
#define TIMPL6803	0x1C00
#define T16DIXE3	0x1D00
#define T16DXE3		0x1E00
#define TDXE3		0x1F00
#define TDIXE3		0x2000
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
#define BADCPU		28
#define TOOMANYJCC	29
#define INVALIDAMODE	30

#elif TARGET_Z8

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_Z8
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS 0			/* For now until CPU type properly settable */

#define TARGET_BIGENDIAN

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

#define ARCH OA_1802
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS 0			/* For now until CPU type properly settable */

#define TARGET_BIGENDIAN

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

#elif TARGET_TMS9995

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_TMS9900
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS OA_TMS9900_9995		/* For now until CPU type properly settable */

#define TARGET_BIGENDIAN
/* We generate intentionally wrapping 16bit maths for relocations */
#define TARGET_RELOC_OVERFLOW_OK

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
#define	TSR	0x0400			/* Special register */
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
#define TDOMA	0x1100			/* addr, addr */
#define TDOMAW	0x1200			/* R, addr */
#define TSMD	0x1300			/* addr */
#define TXOP	0x1400			/* XOP */
#define TSOP	0x1500			/* R */
#define TCRUM	0x1600			/* */
#define TCRUS	0x1700			/* signed offset */
#define TJUMP	0x1800			/* signed offet PC relative */
#define TSHIFT	0x1900			/* reg, num */
#define TIMM	0x1A00			/* reg, num */
#define TIRL	0x1B00			/* addr */
#define TIRLS	0x1C00			/* reg */
#define TEVEN	0x1D00			/* even macro op */
#define TLJUMP	0x1E00			/* jcc/bra expansions */
#define TLJONLY	0x1F00			/* branches always done with jcc bra */

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
#define REG_RANGE	18
#define ADDR_REQUIRED	19
#define INVALID_ID	20
#define BADMODE		21
#define DIVIDE_BY_ZERO	22
#define CONSTANT_RANGE  23
#define DATA_IN_BSS	24
#define SEGMENT_OVERFLOW 25
#define REG_REQUIRED	26
#define	SEGMENT_CLASH	27
#define REG_ZEROONLY	28
#define TOOMANYJCC	29
#define REG_NOTZERO	30
#define ALIGNMENT	31
#define BRA_BAD		32

#elif TARGET_8008

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_8008
#define ARCH_FLAGS 0
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
#define TMADDR	0x00F0			/* Addressing mode bits */

#define TDIRECT	0x0010			/* Direct page */
#define TINDEX	0x0020			/* Indexed */
#define TIMMED	0x0030			/* Immediate */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register */
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
#define TIMM8	0x1100			/* 8bit immediate */
#define TBRA	0x1200			/* Branch */
#define TRST	0x1300			/* RST */
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
#define ADDR_REQUIRED	17
#define INVALID_ID	18
#define BADMODE		19
#define CONSTANT_RANGE  20
#define DATA_IN_BSS	21
#define SEGMENT_OVERFLOW 22
#define	SEGMENT_CLASH	23
#define DIVIDE_BY_ZERO	24

#elif TARGET_SCMP

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_INS8060
#define ARCH_FLAGS 0
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
#define TMADDR	0x00F0			/* Addressing mode bits */

#define TINDEX	0x0010			/* Indexed */
#define TIMMED	0x0020			/* Immediate */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register */
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
#define TMPD	0x1100			/* pointer/offset/autoinc */
#define TPD	0x1200			/* pointer/offset */
#define TIMM8	0x1300			/* 8bit signed */
#define	TREL8	0x1400			/* 8bit relative */
#define TPTR	0x1500			/* pointer */
#define TJS	0x1600			/* special */

#define	P0	0
#define P1	1
#define P2	2
#define P3	3


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
#define ADDR_REQUIRED	17
#define INVALID_ID	18
#define BADMODE		19
#define CONSTANT_RANGE  20
#define DATA_IN_BSS	21
#define SEGMENT_OVERFLOW 22
#define	SEGMENT_CLASH	23
#define DIVIDE_BY_ZERO	24
#define NO_AUTOINDEX	25
#define BRA_RANGE	26
#define RANGE		27
#define INVALIDAMODE	28
#define POINTER_REQ	29

#elif TARGET_WARREX

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_WARREX
#define ARCH_FLAGS OF_BIGENDIAN
#define ARCH_CPUFLAGS OA_WARREX_CPU6

#define TARGET_BIGENDIAN

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
#define	TSR	0x0400			/* Special register (PC) */
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
#define TREL8	0x1100			/* 8bit relative from . + 2 */
/* Assembler extras for resolving branch ranges */
#define TBRA16	0x1200			/* Jcc asm magic */
#define TREGA	0x1300			/* Single accumulator, A short form */
#define TREG	0x1400			/* Single accumulator */
#define TMOVE	0x1500			/* Move.. special */
#define TMMU	0x1600			/* MMU loads */
#define TDMA	0x1700			/* DMA word register */
#define TDMAM	0x1800			/* DMA mode */
#define TREG2A	0x1900			/* Two reg with B,A short forms */
#define TREG2ANWS 0x1A00		/* But without the BX,AX word short */
#define TJUMP	0x1B00			/* Jump/call */
#define TLOAD	0x1C00			/* Load */
#define TSTORE	0x1D00			/* Store */
#define TJUMP8	0x1E00			/* Extending 8bit branch */
#define TREGA8	0x1F00			/* TREGA 8bit only */
#define TREG8	0x2000			/* TREG 8bit only */
#define TREG2A8	0x2100			/* TREG2 8bit only */
#define TMOVE8	0x2200			/* TMOVE 8bit only */
#define TLOADEB	0x2300
#define TLOADEW	0x2400
#define TLOADX	0x2500
#define TSTOREEB 0x2600
#define TSTOREEW 0x2700
#define TSTOREX 0x2800
#define	TSETCPU	0x2900			/* setcpu */
#define TIMPL6	0x2A00			/* Not present on CPU4 */
#define TBLOCK	0x2B00			/* Block operations CPU6 only */

/*
 * Registers.
 */
#define	RA	0
#define RB	2
#define	RX	4
#define	RY	6
#define RZ	8
#define RS	10
#define RC	12
#define	RP	14

#define RAH	0
#define RAL	1
#define RBH	2
#define RBL	3
#define RXH	4
#define RXL	5
#define RYH	6
#define RYL	7
#define RZH	8
#define RZL	9
#define RSH	10
#define RSL	11
#define RCH	12
#define RCL	13
#define RPH	14
#define RPL	15

#define RPC	0


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
#define TOOMANYJCC	28
#define REGONLY		29
#define WREGONLY	30
#define BREGONLY	31
#define REGABBYTE	32
#define REGABXWORD	33
#define BADINDIR	34
#define BADADDR		35
#define RANGE		36
#define AREGONLY	37
#define BADCPU		38

#elif TARGET_BYTECODE

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_BYTE
#define ARCH_FLAGS 0
#define ARCH_CPUFLAGS 0

/* We generate intentionally wrapping 16bit maths for relocations */
#define TARGET_RELOC_OVERFLOW_OK

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
#define	TSETCPU	0x0F00			/* setcpu */
#define	TIMPL	0x1000			/* implicit */
#define	TBYTE	0x1100			/* data byte */
#define	TWORD	0x1200			/* data word */
#define	TLONG	0x1300			/* data long */

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
#define ADDR_REQUIRED	17
#define INVALID_ID	18
#define DIVIDE_BY_ZERO	19
#define CONSTANT_RANGE  20
#define DATA_IN_BSS	21
#define SEGMENT_OVERFLOW 22
#define DATA_IN_ZP	23

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
#define SEGMENT_LIMIT	0xFFFF
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
extern	VALUE   dot[OSEG];
extern  int	segment;
extern	int	debug_write;
extern	char	*fname;
extern  char	*listname;
extern	int	noobj;
extern	int	cpu_flags;

extern int passbegin(int pass);
extern void list_addbyte(uint8_t);
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
extern int outpass(void);
extern void outabsolute(int);
extern void outsegment(int);
extern void outab(uint8_t);
extern void outabyte(uint8_t);
extern void outab2(uint8_t);
extern void outabchk2(uint16_t);
extern void outraw(ADDR *);
extern void outrab(ADDR *);
extern void outrabrel(ADDR *);
extern void outeof(void);
extern void outbyte(uint8_t);
extern void outflush(void);
extern void syminit(void);
extern void reservebyte(void);

extern char *etext[];

#include "obj.h"
