/*
 *	Warrex Centurion
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"a",		TWR,		RA	},
	{	0,	"b",		TWR,		RB	},
	{	0,	"x",		TWR,		RX	},
	{	0,	"y",		TWR,		RY	},
	{	0,	"z",		TWR,		RZ	},
	{	0,	"s",		TWR,		RS	},
	{	0,	"c",		TWR,		RC	},
	{	0,	"p",		TWR,		RP	},

	{	0,	"al",		TBR,		RAL	},
	{	0,	"ah",		TBR,		RAH	},
	{	0,	"bl",		TBR,		RBL	},
	{	0,	"bh",		TBR,		RBH	},
	{	0,	"xl",		TBR,		RXL	},
	{	0,	"xh",		TBR,		RXH	},
	{	0,	"yl",		TBR,		RYL	},
	{	0,	"yh",		TBR,		RYH	},
	{	0,	"zl",		TBR,		RZL	},
	{	0,	"zh",		TBR,		RZH	},
	{	0,	"cl",		TBR,		RCL	},
	{	0,	"ch",		TBR,		RCH	},
	{	0,	"pl",		TBR,		RPL	},
	{	0,	"ph",		TBR,		RPH	},

	/* P is not PC... there are differences */
	{	0,	"pc",		TSR,		RPC	},

	{	0,	"defb",		TDEFB,		XXXX	},
	{	0,	"defw",		TDEFW,		XXXX	},
	{	0,	"defs",		TDEFS,		XXXX	},
	{	0,	"defm",		TDEFM,		XXXX	},
	{	0,	"org",		TORG,		XXXX	},
	{	0,	"equ",		TEQU,		XXXX	},
	{	0,	"export",	TEXPORT,	XXXX	},
	{	0,	".byte",	TDEFB,		XXXX	},
	{	0,	".word",	TDEFW,		XXXX	},
	{	0,	".ds",		TDEFS,		XXXX	},
	{	0,	".ascii",	TDEFM,		XXXX	},
	{	0,	".org",		TORG,		XXXX	},
	{	0,	".equ",		TEQU,		XXXX	},
	{	0,	".export",	TEXPORT,	XXXX	},
	{	0,	".setcpu",	TSETCPU,	XXXX	},
	{	0,	"abs",		TSEGMENT,	ABSOLUTE},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	"literal",	TSEGMENT,	LITERAL },
	{	0,	"commondata",	TSEGMENT,	COMMONDATA },
	{	0,	"buffers",	TSEGMENT,	BUFFERS	},
	{	0,	".abs",		TSEGMENT,	ABSOLUTE},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL },
	{	0,	".commondata",	TSEGMENT,	COMMONDATA },
	{	0,	".buffers",	TSEGMENT,	BUFFERS	},

	/* 0x0X		:	Implicit */
	{	0,	"hlt",		TIMPL,		0x00	},
	{	0,	"nop",		TIMPL,		0x01	},
	{	0,	"sf",		TIMPL,		0x02	},
	{	0,	"rf",		TIMPL,		0x03	},
	{	0,	"ei",		TIMPL,		0x04	},
	{	0,	"di",		TIMPL,		0x05	},
	{	0,	"sl",		TIMPL,		0x06	},
	{	0,	"rl",		TIMPL,		0x07	},
	{	0,	"cl",		TIMPL,		0x08	},
	{	0,	"rsr",		TIMPL,		0x09	},
	{	0,	"ri",		TIMPL,		0x0A	},
	{	0,	"rim",		TIMPL,		0x0B	},
	{	0,	"elo",		TIMPL,		0x0C	},
	{	0,	"pcx",		TIMPL,		0x0D	},
	{	0,	"dly",		TIMPL,		0x0E	},
	{	0,	"sysret",	TIMPL6,		0x0F	},

	/* 0x1X		:	Branches */
#if 0
	/* Clashes with the bl register so handle in code */
	{	0,	"bl",		TREL8,		0x10	},
#endif
	{	0,	"bnl",		TREL8,		0x11	},
	{	0,	"bf",		TREL8,		0x12	},
	{	0,	"bnf",		TREL8,		0x13	},
	{	0,	"bz",		TREL8,		0x14	},
	{	0,	"bnz",		TREL8,		0x15	},
	{	0,	"bm",		TREL8,		0x16	},
	{	0,	"bp",		TREL8,		0x17	},
	{	0,	"bgz",		TREL8,		0x18	},
	{	0,	"ble",		TREL8,		0x19	},
	{	0,	"bs1",		TREL8,		0x1A	},
	{	0,	"bs2",		TREL8,		0x1B	},
	{	0,	"bs3",		TREL8,		0x1C	},
	{	0,	"bs4",		TREL8,		0x1D	},
	{	0,	"btm",		TREL8,		0x1E	},
	{	0,	"bep",		TREL8,		0x1F	},
	
	/* Special magic forms : these will be smart one day */
	{	0,	"jc",		TBRA16,		0x10	},
	{	0,	"jcc",		TBRA16,		0x11	},
	{	0,	"jn",		TBRA16,		0x12	},
	{	0,	"jnc",		TBRA16,		0x13	},
	{	0,	"jz",		TBRA16,		0x14	},
	{	0,	"jnz",		TBRA16,		0x15	},
	{	0,	"jlt",		TBRA16,		0x16	},
	{	0,	"jge",		TBRA16,		0x17	},
	{	0,	"jle",		TBRA16,		0x18	},
	{	0,	"jgt",		TBRA16,		0x19	},
	{	0,	"js1",		TBRA16,		0x1A	},
	{	0,	"js2",		TBRA16,		0x1B	},
	{	0,	"js3",		TBRA16,		0x1C	},
	{	0,	"js4",		TBRA16,		0x1D	},
	{	0,	"bra",		TJUMP8,		0x73	},
	{	0,	"bsr",		TJUMP8,		0x7B	},
	

	/* 0x2X		:	Mix of ALU and misc */
	/* Short forms apply to AL, long forms any register */
	/* 0x3x versions are word and apply to A, long to any register */
	/* TODO: add inrb etc forms for specific 8bit */

	{	0,	"inr",		TREGA,		0x20	},
	{	0,	"dcr",		TREGA,		0x21	},
	{	0,	"clr",		TREGA,		0x22	},
	{	0,	"ivr",		TREGA,		0x23	},
	{	0,	"srr",		TREGA,		0x24	},
	{	0,	"slr",		TREGA,		0x25	},
	{	0,	"rrr",		TREG,		0x26	},
	{	0,	"rlr",		TREG,		0x27	},

	{	0,	"inrb",		TREGA8,		0x20	},
	{	0,	"dcrb",		TREGA8,		0x21	},
	{	0,	"clrb",		TREGA8,		0x22	},
	{	0,	"ivrb",		TREGA8,		0x23	},
	{	0,	"srrb",		TREGA8,		0x24	},
	{	0,	"slrb",		TREGA8,		0x25	},
	{	0,	"rrrb",		TREG8,		0x26	},
	{	0,	"rlrb",		TREG8,		0x27	},

	/* Names used on the EE200 for short forms */
	{	0,	"ina",		TIMPL,		0x38	},
	{	0,	"inab",		TIMPL,		0x28	},
	{	0,	"dca",		TIMPL,		0x39	},
	{	0,	"dcab",		TIMPL,		0x29	},
	{	0,	"cla",		TIMPL,		0x3A	},
	{	0,	"clab",		TIMPL,		0x2A	},
	{	0,	"iva",		TIMPL,		0x3B	},
	{	0,	"ivab",		TIMPL,		0x2B	},
	{	0,	"sra",		TIMPL,		0x3C	},
	{	0,	"srab",		TIMPL,		0x2C	},
	{	0,	"sla",		TIMPL,		0x3D	},
	{	0,	"slab",		TIMPL,		0x2D	},
	{	0,	"inx",		TIMPL,		0x3E	},
	{	0,	"dcx",		TIMPL,		0x3F	},

	/* 0x2E/F	:	Special */

	{	0,	"ldmmu",	TMMU,		0x2E0C  },
	{	0,	"stmmu",	TMMU,		0x2E1C	},
	{	0,	"stdma",	TDMA,		0x2F00	},
	{	0,	"lddma",	TDMA,		0x2F01	},
	{	0,	"stdmac",	TDMA,		0x2F02	},
	{	0,	"lddmac",	TDMA,		0x2F03	},
	{	0,	"dmamode",	TDMAM,		0x2F04	},
	{	0,	"dmaen",	TIMPL,		0x2F06	},
	
	/* 0x4X		:	ALU operations, short are AL,BL */
	/* 0x5X		:	ALU operations, short are AX,BX */

	{	0,	"add",		TREG2A,		0x40	},
	{	0,	"sub",		TREG2A,		0x41	},
	{	0,	"and",		TREG2A,		0x42	},
	/* These forms have no short word version */
	{	0,	"ori",		TREG2ANWS,	0x43	},
	{	0,	"ore",		TREG2ANWS,	0x44	},

	{	0,	"addb",		TREG2A8,	0x40	},
	{	0,	"subb",		TREG2A8,	0x41	},
	{	0,	"andb",		TREG2A8,	0x42	},
	{	0,	"orib",		TREG2A8,	0x43	},
	{	0,	"oreb",		TREG2A8,	0x44	},

	/* Special 0x47: forms we know */

	{	0,	"bcpy",		TBLOCK,		0x4740	},
	{	0,	"bcmp",		TBLOCK,		0x4780	},

	/* Move ("xfr") handled specially */
	{	0,	"xfr",		TMOVE,		0x00	},
	{	0,	"xfrb",		TMOVE,		0x00	},

	/* EE200 names for short forms */
	{	0,	"aab",		TIMPL,		0x58	},
	{	0,	"aabb",		TIMPL,		0x48	},
	{	0,	"sab",		TIMPL,		0x59	},
	{	0,	"sabb",		TIMPL,		0x49	},
	{	0,	"nab",		TIMPL,		0x5A	},
	{	0,	"nabb",		TIMPL,		0x4A	},
	{	0,	"xax",		TIMPL,		0x5B	},
	{	0,	"xaxb",		TIMPL,		0x4B	},
	{	0,	"xay",		TIMPL,		0x5C	},
	{	0,	"xayb",		TIMPL,		0x4C	},
	{	0,	"xab",		TIMPL,		0x5D	},
	{	0,	"xabb",		TIMPL,		0x4D	},
	{	0,	"xaz",		TIMPL,		0x5E	},
	{	0,	"xazb",		TIMPL,		0x4E	},
	{	0,	"xas",		TIMPL,		0x5F	},
	{	0,	"xasb",		TIMPL,		0x4F	},

	/* 0x6X		; 	Entirely word moves with X	*/

	/* 0x7X		:	Calls and other */
	{	0,	"jmp",		TJUMP,		0x70	},
	{	0,	"jsr",		TJUMP,		0x78	},
	{	0,	"syscall",	TIMPL6,		0x76	},
	
	/* 0x8x-0xFF	:	A and B load/store */

	{	0,	"ld",		TLOAD,		0x00	},
	{	0,	"st",		TSTORE,		0x00	},

	/* EE200 forms */
	{	0,	"lda",		TLOADEW,	0x90	},
	{	0,	"ldab",		TLOADEB,	0x80	},
	{	0,	"ldb",		TLOADEW,	0xD0	},
	{	0,	"ldbb",		TLOADEB,	0xC0	},
	{	0,	"ldx",		TLOADX,		0x60	},
	{	0,	"sta",		TSTOREEW,	0xB0	},
	{	0,	"stab",		TSTOREEB,	0xA0	},
	{	0,	"stb",		TSTOREEW,	0xF0	},
	{	0,	"stbb",		TSTOREEB,	0xE0	},
	{	0,	"stx",		TSTOREX,	0x68	},
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
	"too many Jcc instructions",	/* 28 */
	"register required",		/* 29 */
	"word register required",	/* 30 */
	"byte register required",	/* 31 */
	"AL or BL only",		/* 32 */
	"A B or X only",		/* 33 */
	"Can't indirect",		/* 34 */
	"Invalid addressing mode",	/* 35 */
	"Out of range",			/* 36 */
	"A register only",		/* 37 */
	"Not available on CPU4",	/* 38 */
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
