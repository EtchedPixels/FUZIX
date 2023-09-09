/*
 * Z-80 assembler.
 * Basic symbol tables.
 * Contain all of the instructions
 * and register names.
 */
#include	"as.h"

/*
 * This array of symbol nodes
 * make up the basic symbol table.
 * The "syminit" routine links these
 * nodes into the builtin symbol hash
 * table at start-up time.
 */
SYM	sym[] = {
	{	0,	"a",		TBR,		A	},
	{	0,	"b",		TBR,		B	},
	{	0,	"dp",		TBR,		DP	},
	{	0,	"cc",		TBR,		CC	},
	{	0,	"d",		TWR,		D	},
	{	0,	"u",		TWR,		U	},
	{	0,	"x",		TWR,		X	},
	{	0,	"y",		TWR,		Y	},
	{	0,	"s",		TWR,		S	},
	{	0,	"pc",		TWR,		PCR	},
	{	0,	"pcr",		TWR,		PCR	},
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
	{	0,	"cond",		TCOND,		XXXX	},
	{	0,	"endc",		TENDC,		XXXX	},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".dp",		TSEGMENT,	ZP	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},

	{	0,	"adca",		THI,		0x89	},
	{	0,	"adcb",		THI,		0xC9	},
	{	0,	"adda",		THI,		0x8B	},
	{	0,	"addb",		THI,		0xCB	},
	{	0,	"addd",		THIW,		0xC3	},
	{	0,	"anda",		THI,		0x84	},
	{	0,	"andb",		THI,		0xC4	},
	{	0,	"adcc",		TIMM8,		0x1C	},
	{	0,	"asla",		TIMP,		0x48	},
	{	0,	"aslb",		TIMP,		0x58	},
	{	0,	"asl",		TLO,		0x08	},
	{	0,	"asra",		TIMP,		0x47	},
	{	0,	"asrb",		TIMP,		0x57	},
	{	0,	"asr",		TLO,		0x07	},
	/* TODO Bxx */
	{	0,	"bita",		THI,		0x85	},
	{	0,	"bitb",		THI,		0xC5	},
	{	0,	"clra",		TIMP,		0x4F	},
	{	0,	"clrb",		TIMP,		0x5F	},
	{	0,	"clr",		TLO,		0x0F	},
	{	0,	"cmpa",		THI,		0x81	},
	{	0,	"cmpb",		THI,		0xC1	},
	{	0,	"cmpd",		THIW,		0x1083	},
	{	0,	"cmps",		THIW,		0x118C	},
	{	0,	"cmpu",		THIW,		0x1183	},
	{	0,	"cmpx",		THIW,		0x8C	},
	{	0,	"cmpy",		THIW,		0x108C	},
	{	0,	"coma",		TIMP,		0x43	},
	{	0,	"comb",		TIMP,		0x53	},
	{	0,	"com",		TLO,		0x03	},
	{	0,	"cwai",		TIMM8,		0x3C	},
	{	0,	"daa",		TIMP,		0x19	},
	{	0,	"deca",		TIMP,		0x4A	},
	{	0,	"decb",		TIMP,		0x5A	},
	{	0,	"dec",		TLO,		0x08	},
	{	0,	"eora",		THI,		0x88	},
	{	0,	"eorb",		THI,		0xC8	},
	{	0,	"exg",		TEXG,		0x1E	},
	{	0,	"inca",		TIMP,		0x4C	},
	{	0,	"incb",		TIMP,		0x5C	},
	{	0,	"inc",		TLO,		0x0C	},
	{	0,	"jmp",		TLO,		0x0E	},
	{	0,	"jsr",		TLO,		0x9D	},
	/* TODO: lbxx */
	{	0,	"lda",		THI,		0x86	},
	{	0,	"ldb",		THI,		0xC6	},
	{	0,	"ldd",		THIW,		0xCC	},
	{	0,	"lds",		THIW,		0x10CE	},
	{	0,	"ldu",		THIW,		0xCE	},
	{	0,	"ldx",		THIW,		0x8E	},
	{	0,	"ldy",		THIW,		0x108E	},
	{	0,	"leas",		TLEA,		0x32	},
	{	0,	"leau",		TLEA,		0x33	},
	{	0,	"leax",		TLEA,		0x30	},
	{	0,	"leay",		TLEA,		0x31	},
	{	0,	"lsla",		TIMP,		0x48	},
	{	0,	"lslb",		TIMP,		0x49	},
	{	0	"lsl",		TLO,		0x08	},
	{	0,	"lsra",		TIMP,		0x44	},
	{	0,	"lsrb",		TIMP,		0x54	},
	{	0,	"lsr",		TLO,		0x04	},
	{	0,	"mul",		TIMP,		0x3D	},
	{	0,	"nega",		TIMP,		0x40	},
	{	0,	"negb",		TIMP,		0x50	},
	{	0,	"neg",		TLO,		0x00	},
	{	0,	"nop",		TIMP,		0x12	},
	{	0,	"ora",		THI,		0x8A	},
	{	0,	"orb",		THI,		0xCA	},
	{	0,	"orcc",		TIMM8,		0x1A	},
	{	0,	"pshs",		TPUSH,		0x34	},
	{	0,	"pshu",		TPUSH,		0x36	},
	{	0,	"puls",		TPUSH,		0x35	},
	{	0,	"pulu",		TPUSH,		0x37	},
	{	0,	"rola",		TIMP,		0x49	},
	{	0,	"rolb",		TIMP,		0x59	},
	{	0,	"rol",		TLO,		0x09	},
	{	0,	"rora",		TIMP,		0x46	},
	{	0,	"rorb",		TIMP,		0x56	},
	{	0,	"ror",		TLO,		0x06	},
	{	0,	"rti",		TIMP,		0x3B	},
	{	0,	"rts",		TIMP,		0x39	},
	{	0,	"sbca",		THI,		0x82	},
	{	0,	"sbcb",		THI,		0xC2	},
	{	0,	"sex",		TIMP,		0x1D	},
	{	0,	"sta",		THINOIMM,	0x87	},
	{	0,	"stb",		THINOIMM,	0xC7	},
	{	0,	"std",		THIWNOIMM,	0xCD	},
	{	0,	"sts",		THIWNOIMM,	0x10CF	},
	{	0,	"stu",		THIWNOIMM,	0xCF	},
	{	0,	"stx",		THIWNOIMM,	0x8F	},
	{	0,	"sty",		THIWNOIMM,	0x108F	},
	{	0,	"suba",		THI,		0x80	},
	{	0,	"subb",		THI,		0xC0	},
	{	0,	"subd",		THIW,		0x83	},
	{	0,	"swi",		TIMP,		0x3F	},
	{	0,	"swi2",		TIMP,		0x103F	},
	{	0,	"swi3",		TIMP,		0x113F	},
	{	0,	"sync",		TIMP,		0x13	},
	{	0,	"tfr",		TEXG,/*check*/	0x1F	},
	{	0,	"tsta",		TIMP,		0x4D	},
	{	0,	"tstb",		TIMP,		0x5D	},
	{	0	"tst",		TLO,		0x0D	},
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
	"unexpected character",
	"phase error",
	"multiple definitions",
	"syntax error",
	"must be absolute",
	"missing delimiter",
	"invalid constant",
	"JR out of range",
	"condition required",
	"invalid register for operation",
	"address required",
	"invalid id",
	"must be C",
	"divide by 0",
	"constant out of range",
	"data in BSS",
	"segment overflow",
	"Z180 instruction",
	"segment conflict"
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
	int reg;

	mode = ap->a_type&TMMODE;
	if (mode == TUSER)
		return;
	if (mode==TWR && paren!=0) {
		reg = ap->a_type&TMREG;
		if (reg==IX || reg==IY)
			return;
	}
	aerr(ADDR_REQUIRED);
}
