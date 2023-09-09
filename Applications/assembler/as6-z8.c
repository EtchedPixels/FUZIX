/*
 * Z8 assembler.

 *
 * Tables: FIXME -  we want an address space of 'register' ?
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"defb",		TDEFB,		XXXX	},
	{	0,	"defw",		TDEFW,		XXXX	},
	{	0,	"defs",		TDEFS,		XXXX	},
	{	0,	"defm",		TDEFM,		XXXX	},
	{	0,	"org",		TORG,		XXXX	},
	{	0,	"equ",		TEQU,		XXXX	},
	{	0,	"export",	TEXPORT,	XXXX	},
	{	0,	".byte",	TDEFB,		XXXX	},
	{	0,	".word",	TDEFW,		XXXX	},
	{	0,	".blkb",	TDEFS,		XXXX	},
	{	0,	".ascii",	TDEFM,		XXXX	},
	{	0,	".org",		TORG,		XXXX	},
	{	0,	".equ",		TEQU,		XXXX	},
	{	0,	".export",	TEXPORT,	XXXX	},
	{	0,	"abs",		TSEGMENT,	ABSOLUTE},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"zp",		TSEGMENT,	ZP	},
	{	0,	".abs",		TSEGMENT,	ABSOLUTE},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".zp",		TSEGMENT,	ZP	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	
	/* Condition codes */
	
	{	0,	"f",		TCC,		0x00	},
	{	0,	"lt",		TCC,		0x01	},
	{	0,	"le",		TCC,		0x02	},
	{	0,	"ule",		TCC,		0x03	},
	{	0,	"ov",		TCC,		0x04	},
	{	0,	"mi",		TCC,		0x05	},
	{	0,	"eq",		TCC,		0x06	},
	{	0,	"z",		TCC,		0x06	},
	{	0,	"c",		TCC,		0x07	},
	/* 0x08 is 'true' */
	{	0,	"ge",		TCC,		0x09	},
	{	0,	"gt",		TCC,		0x0A	},
	{	0,	"uge",		TCC,		0x0B	},
	{	0,	"nov",		TCC,		0x0C	},
	{	0,	"pl",		TCC,		0x0D	},
	{	0,	"ne",		TCC,		0x0E	},
	{	0,	"nz",		TCC,		0x0E	},
	{	0,	"nc",		TCC,		0x0F	},

	/* The Z8 has a gloriously clean instruction set */
	
	{	0,	"add",		TOP4BIT,	0x00	},
	{	0,	"adc",		TOP4BIT,	0x10	},
	{	0,	"sub",		TOP4BIT,	0x20	},
	{	0,	"sbc",		TOP4BIT,	0x30	},
	{	0,	"or",		TOP4BIT,	0x40	},
	{	0,	"and",		TOP4BIT,	0x50	},
	{	0,	"tcm",		TOP4BIT,	0x60	},
	{	0,	"tm",		TOP4BIT,	0x70	},
	/* 8-9 are LD */
	{	0,	"cp",		TOP4BIT,	0xA0	},
	{	0,	"xor",		TOP4BIT,	0xB0	},
	/* C is LD */


	{	0,	"jr",		TCRA,		0x0B	},
	/* jp is an oddity as it's got two real forms 30 and xD */
	{	0,	"jp",		TJMP,		0x0D	},
	{	0,	"inc",		TRRIR,		0x0E	},

	/* Implicit */	
	{	0,	"di",		TIMPL,		0x8F	},
	{	0,	"ei",		TIMPL,		0x9F	},
	{	0,	"ret",		TIMPL,		0xAF	},
	{	0,	"iret",		TIMPL,		0xBF	},
	{	0,	"rcf",		TIMPL,		0xCF	},
	{	0,	"scf",		TIMPL,		0xDF	},
	{	0,	"ccf",		TIMPL,		0xEF	},
	{	0,	"nop",		TIMPL,		0xFF	},

	/* R / IR */
	{	0,	"dec",		TRIR,		0x00	},
	{	0,	"rlc",		TRIR,		0x10	},
	/* 30 is used for jump IRR */
	{	0,	"da",		TRIR,		0x40	},
	{	0,	"pop",		TRIR,		0x50	},
	{	0,	"com",		TRIR,		0x60	},
	{	0,	"push",		TRIR,		0x70	},
	{	0,	"decw",		TRRIR,		0x80	},
	{	0,	"rl",		TRIR,		0x90	},
	{	0,	"incw",		TRRIR,		0xA0	},
	{	0,	"clr",		TRIR,		0xB0	},
	{	0,	"rrc",		TRIR,		0xC0	},
	{	0,	"sra",		TRIR,		0xD0	},
	{	0,	"rr",		TRIR,		0xE0	},
	{	0,	"swap",		TRIR,		0xF0	},

	/* The less regular bits */
	{	0,	"srp",		TIMM8,		0x31	},
	/* FIXME: call DA on later devices (D6) */
	{	0,	"call",		TIRRDA,		0xD4	},
	{	0,	"djnz",		TRA,		0x0A	},
	{	0,	"ldc",		TLDC,		0xC2	},
	{	0,	"ldci",		TLDCI,		0xC3	},
	{	0,	"lde",		TLDC,		0x82	},
	{	0,	"ldei",		TLDCI,		0x83	},

	/* And load which has a small army of forms */
	{	0,	"ld",		TLOAD,		0x00	},
	
};

        
/*
 * Set up the symbol table.
 * Sweep through the initializations
 * of the "phash", and link them into the
 * buckets. Because it is here, a
 * "sizeof" works.
 */
void syminit(void)
{
	SYM *sp;
	int hash;

	sp = &sym[0];
	while (sp < &sym[sizeof(sym)/sizeof(SYM)]) {
		hash = symhash(sp->s_id);
		sp->s_fp = phash[hash];
		phash[hash] = sp;
		++sp;
	}
}

char *etext[] = {
	"unexpected character",		/* 10 */
	"phase error",			/* 11 */
	"multiple definitions",		/* 12 */
	"syntax error",			/* 13 */
	"must be absolute",		/* 14 */
	"missing delimiter",		/* 15 */
	"invalid constant",		/* 16 */
	"Bcc out of range",		/* 17 */
	"register 0-15 required",	/* 18 */
	"address required",		/* 19 */
	"invalid ID",			/* 20 */
	"invalid instruction form",	/* 21 */
	"divide by 0",			/* 22 */
	"constant out of range",	/* 23 */
	"data in BSS",			/* 24 */
	"segment overflow",		/* 25 */
	"data in ZP",			/* 26 */
	"segment conflict",		/* 27 */
	"register must be even"		/* 28 */
};

/*
 * Make sure that the
 * mode and register fields of
 * the type of the "ADDR" pointed to
 * by "ap" can participate in an addition
 * or a subtraction.
 */
void isokaors(ADDR *ap, int paren)
{
	int mode;

	mode = ap->a_type&TMMODE;
	if (mode == TUSER)
		return;
	aerr(ADDR_REQUIRED);
}
