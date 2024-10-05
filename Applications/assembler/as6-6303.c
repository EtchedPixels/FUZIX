/*
 * 6303 assembler.
 *
 * Tables
 *
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"a",		TBR,		A	},
	{	0,	"b",		TBR,		B	},
	{	0,	"d",		TWR,		D	},
	{	0,	"x",		TWR,		X	},

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
	{	0,	".ds",		TDEFS,		XXXX	},
	{	0,	".ascii",	TDEFM,		XXXX	},
	{	0,	".org",		TORG,		XXXX	},
	{	0,	".equ",		TEQU,		XXXX	},
	{	0,	".export",	TEXPORT,	XXXX	},
	{	0,	"abs",		TSEGMENT,	ABSOLUTE},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	"zp",		TSEGMENT,	ZP	},
	{	0,	".abs",		TSEGMENT,	ABSOLUTE},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".zp",		TSEGMENT,	ZP	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	{	0,	".commondata",	TSEGMENT,	COMMONDATA },
	{	0,	".buffers",	TSEGMENT,	BUFFERS	},
	{	0,	".setcpu",	TSETCPU,	XXXX	},

	/* 0x0X		:	Implicit */
	{	0,	"nop",		TIMPL,		0x01	},
	{	0,	"lsrd",		TIMPL6803,	0x04	},
	{	0,	"asld",		TIMPL6803,	0x05	},
	{	0,	"lsld",		TIMPL6803,	0x05	},
	{	0,	"tap",		TIMPL,		0x06	},
	{	0,	"tpa",		TIMPL,		0x07	},
	{	0,	"inx",		TIMPL,		0x08	},
	{	0,	"dex",		TIMPL,		0x09	},
	{	0,	"clv",		TIMPL,		0x0A	},
	{	0,	"sev",		TIMPL,		0x0B	},
	{	0,	"clc",		TIMPL,		0x0C	},
	{	0,	"sec",		TIMPL,		0x0D	},
	{	0,	"cli",		TIMPL,		0x0E	},
	{	0,	"sei",		TIMPL,		0x0F	},

	/* 0x1X		:	Implicit */
	{	0,	"sba",		TIMPL,		0x10	},
	{	0,	"cba",		TIMPL,		0x11	},
	{	0,	"tab",		TIMPL,		0x16	},
	{	0,	"tba",		TIMPL,		0x17	},
	{	0,	"daa",		TIMPL,		0x19	},
	{	0,	"aba",		TIMPL,		0x1B	},

	/* 0x1X		:	Implicit 6303 */
	{	0,	"xgdx",		TIMPL6303,	0x18	},
	{	0,	"slp",		TIMPL6303,	0x1A	},

	/* 0x2X		:	Branches */
	{	0,	"bra",		TREL8,		0x20	},
	{	0,	"brn",		TREL8,		0x21	},
	{	0,	"bhi",		TREL8,		0x22	},
	{	0,	"bls",		TREL8,		0x23	},
	{	0,	"bcc",		TREL8,		0x24	},
	{	0,	"bhs",		TREL8,		0x24	},
	{	0,	"bcs",		TREL8,		0x25	},
	{	0,	"blo",		TREL8,		0x25	},
	{	0,	"bne",		TREL8,		0x26	},
	{	0,	"beq",		TREL8,		0x27	},
	{	0,	"bvc",		TREL8,		0x28	},
	{	0,	"bvs",		TREL8,		0x29	},
	{	0,	"bpl",		TREL8,		0x2A	},
	{	0,	"bmi",		TREL8,		0x2B	},
	{	0,	"bge",		TREL8,		0x2C	},
	{	0,	"blt",		TREL8,		0x2D	},
	{	0,	"bgt",		TREL8,		0x2E	},
	{	0,	"ble",		TREL8,		0x2F	},
	
	/* Special magic forms : these will be smart one day */
	{	0,	"jhi",		TBRA16,		0x22	},
	{	0,	"jls",		TBRA16,		0x23	},
	{	0,	"jcc",		TBRA16,		0x24	},
	{	0,	"jhs",		TBRA16,		0x24	},
	{	0,	"jcs",		TBRA16,		0x25	},
	{	0,	"jlo",		TBRA16,		0x25	},
	{	0,	"jne",		TBRA16,		0x26	},
	{	0,	"jeq",		TBRA16,		0x27	},
	{	0,	"jvc",		TBRA16,		0x28	},
	{	0,	"jvs",		TBRA16,		0x29	},
	{	0,	"jpl",		TBRA16,		0x2A	},
	{	0,	"jmi",		TBRA16,		0x2B	},
	{	0,	"jge",		TBRA16,		0x2C	},
	{	0,	"jlt",		TBRA16,		0x2D	},
	{	0,	"jgt",		TBRA16,		0x2E	},
	{	0,	"jle",		TBRA16,		0x2F	},
	

	/* 0x3X		:	Implicit */
	{	0,	"tsx",		TIMPL,		0x30	},
	{	0,	"ins",		TIMPL,		0x31	},
	{	0,	"pula",		TIMPL,		0x32	},
	{	0,	"pulb",		TIMPL,		0x33	},
	{	0,	"des",		TIMPL,		0x34	},
	{	0,	"txs",		TIMPL,		0x35	},
	{	0,	"psha",		TIMPL,		0x36	},
	{	0,	"pshb",		TIMPL,		0x37	},
	{	0,	"pulx",		TIMPL6803,	0x38	},
	{	0,	"rts",		TIMPL,		0x39	},
	{	0,	"abx",		TIMPL6803,	0x3A	},
	{	0,	"rti",		TIMPL,		0x3B	},
	{	0,	"pshx",		TIMPL6803,	0x3C	},
	{	0,	"mul",		TIMPL6803,	0x3D	},
	{	0,	"wai",		TIMPL,		0x3E	},
	{	0,	"swi",		TIMPL,		0x3F	},
	
	/* 0x4X		:	Implicit ops on A */

	{	0,	"nega",		TIMPL,		0x40	},
	{	0,	"coma",		TIMPL,		0x43	},
	{	0,	"lsra",		TIMPL,		0x44	},
	{	0,	"rora",		TIMPL,		0x46	},
	{	0,	"asra",		TIMPL,		0x47	},
	{	0,	"asla",		TIMPL,		0x48	},
	{	0,	"lsla",		TIMPL,		0x48	},
	{	0,	"rola",		TIMPL,		0x49	},
	{	0,	"deca",		TIMPL,		0x4A	},
	{	0,	"inca",		TIMPL,		0x4C	},
	{	0,	"tsta",		TIMPL,		0x4D	},
	/* 4E is HCF but has no official code */
	{	0,	"clra",		TIMPL,		0x4F	},

	/* 0x5X		:	Implicit ops on B */

	{	0,	"negb",		TIMPL,		0x50	},
	{	0,	"comb",		TIMPL,		0x53	},
	{	0,	"lsrb",		TIMPL,		0x54	},
	{	0,	"rorb",		TIMPL,		0x56	},
	{	0,	"asrb",		TIMPL,		0x57	},
	{	0,	"aslb",		TIMPL,		0x58	},
	{	0,	"lslb",		TIMPL,		0x58	},
	{	0,	"rolb",		TIMPL,		0x59	},
	{	0,	"decb",		TIMPL,		0x5A	},
	{	0,	"incb",		TIMPL,		0x5C	},
	{	0,	"tstb",		TIMPL,		0x5D	},
	/* 5E is HCF but has no official code */
	{	0,	"clrb",		TIMPL,		0x5F	},

	/* 0x6X/0x7X	:	Indexed and extended */

	{	0,	"neg",		TXE,		0x60	},
	{	0,	"com",		TXE,		0x63	},
	{	0,	"lsr",		TXE,		0x64	},
	{	0,	"ror",		TXE,		0x66	},
	{	0,	"asr",		TXE,		0x67	},
	{	0,	"asl",		TXE,		0x68	},
	{	0,	"lsl",		TXE,		0x68	},
	{	0,	"rol",		TXE,		0x69	},
	{	0,	"dec",		TXE,		0x6A	},
	{	0,	"inc",		TXE,		0x6C	},
	{	0,	"tst",		TXE,		0x6D	},
	{	0,	"jmp",		TXE,		0x6E	},
	{	0,	"clr",		TXE,		0x6F	},
	
	/* 0x6X/0x7X	:	6303 specific Imm/Direct and Imm/Index */
	
	{	0,	"aim",		TIDX6303,	0x61	},
	{	0,	"oim",		TIDX6303,	0x62	},
	{	0,	"eim",		TIDX6303,	0x65	},
	{	0,	"tim",		TIDX6303,	0x6B	},
	
	/* 0x6X/0x7X	:	Aliases to above with bit syntax */
	{	0,	"bclr",		TIDXB6303,	0xFF61	},
	{	0,	"bset",		TIDXB6303,	0x62	},
	{	0,	"btgl",		TIDXB6303,	0x65	},
	{	0,	"btst",		TIDXB6303,	0x6B	},

	/* 0x8x-0xBx	:	A and S form load/store/logic/oddities */

	{	0,	"suba",		TDIXE,		0x80	},
	{	0,	"cmpa",		TDIXE,		0x81	},
	{	0,	"sbca",		TDIXE,		0x82	},
	{	0,	"subd",		T16DIXE3,	0x83	},
	{	0,	"anda",		TDIXE,		0x84	},
	{	0,	"bita",		TDIXE,		0x85	},
	{	0,	"ldaa",		TDIXE,		0x86	},
	{	0,	"staa",		TDXE,		0x87	},
	{	0,	"eora",		TDIXE,		0x88	},
	{	0,	"adca",		TDIXE,		0x89	},
	{	0,	"oraa",		TDIXE,		0x8A	},
	{	0,	"adda",		TDIXE,		0x8B	},
	{	0,	"cpx",		T16DIXE,	0x8C	},
	{	0,	"lds",		T16DIXE,	0x8E	},
	{	0,	"sts",		T16DXE,		0x8F	},
	/* Now the oddities between 0x80-0xBF */
	{	0,	"bsr",		TREL8,		0x8D	},
	{	0,	"jsr",		T16DXE,		0x8D	},
	
	/* 0xCx-0xFx	:	B, D and X form load/store/logic/oddities */
	
	{	0,	"subb",		TDIXE,		0xC0	},
	{	0,	"cmpb",		TDIXE,		0xC1	},
	{	0,	"sbcb",		TDIXE,		0xC2	},
	{	0,	"addd",		T16DIXE,	0xC3	},
	{	0,	"andb",		TDIXE,		0xC4	},
	{	0,	"bitb",		TDIXE,		0xC5	},
	{	0,	"ldab",		TDIXE,		0xC6	},
	{	0,	"stab",		TDXE,		0xC7	},
	{	0,	"eorb",		TDIXE,		0xC8	},
	{	0,	"adcb",		TDIXE,		0xC9	},
	{	0,	"orab",		TDIXE,		0xCA	},
	{	0,	"addb",		TDIXE,		0xCB	},
	{	0,	"ldd",		T16DIXE3,	0xCC	},
	{	0,	"std",		T16DIXE3,	0xCD	},
	{	0,	"ldx",		T16DIXE,	0xCE	},
	{	0,	"stx",		T16DXE,		0xCF	},
	{	0,	"jsr",		T16DXE,		0x8D	},
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
	"index out of range",		/* 18 */
	"address required",		/* 19 */
	"invalid ID",			/* 20 */
	"bad addressing mode",		/* 21 */
	"divide by 0",			/* 22 */
	"constant out of range",	/* 23 */
	"data in BSS",			/* 24 */
	"segment overflow",		/* 25 */
	"data in direct page",		/* 26 */
	"segment conflict",		/* 27 */
	"unsupported by this CPU",	/* 28 */
	"too many Jcc instructions",	/* 29 */
	"invalid addressing mode"	/* 30 */
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
