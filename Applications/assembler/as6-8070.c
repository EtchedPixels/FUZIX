/*
 * SCP/MP
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
	{	0,	"pc",		TBR,		P0	},
	{	0,	"sp",		TBR,		P1	},
	{	0,	"p0",		TBR,		P0	},
	{	0,	"p1",		TBR,		P1	},
	{	0,	"p2",		TBR,		P2	},
	{	0,	"p3",		TBR,		P3	},
	{	0,	"a",		TBR,		A	},
	{	0,	"ea",		TBR,		EA	},
	{	0,	"e",		TBR,		E	},
	{	0,	"s",		TBR,		S	},
	{	0,	"t",		TBR,		T	},
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
	{	0,	"cond",		TCOND,		XXXX	},
	{	0,	"endc",		TENDC,		XXXX	},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},

	/* Implicit */
	{	0,	"nop",		TIMPL,		0x00	},
	{	0,	"ret",		TIMPL,		0x5C	},
	
	/* SSM (P2/P3 only) */
	{	0,	"ssm",		TP2P3,		0x2C	},

	/* 16bit const - really JSR is PLI 0 and jmp is LD PC,=addr */
	{	0,	"jsr",		TIMM16,		0x20	},
	{	0,	"jmp",		TIMM16,		0x24	},

	/* PLI (p, imm16) */
	{	0,	"pli",		TPIMM16,	0x20	},

	/* logic 8 */
	{	0,	"and",		TLOGIC,		0x39D0	},
	{	0,	"or",		TLOGIC,		0x3BD8	},
	{	0,	"xor",		TLOGIC,		0x00E0	},

	/* logic16 */
	{	0,	"add",		TLOGIC16,	0xF0B0	},
	{	0,	"sub",		TLOGIC16,	0xF8B8	},

	/* mem8 */
	{	0,	"dld",		TMEM8,		0x98	},
	{	0,	"ild",		TMEM8,		0x90	},

	/* branches */
	{	0,	"bnd",		TBND,		0x2D	},
	{	0,	"bp",		TBRA,		0x64	},
	{	0,	"bz",		TBRA,		0x6C	},
	{	0,	"bra",		TBRA,		0x74	},
	{	0,	"bnz",		TBRA,		0x8C	},

	/* exchange */
	{	0,	"xch",		TXCH,		0x44	},

	/* call */
	{	0,	"call",		TCALL,		0x10	},

	/* eat/t */
	{	0,	"div",		TEAT,		0x0D	},
	{	0,	"mpy",		TEAT,		0x2C	},

	/* Load and store */
	{	0,	"ld",		TLOAD,		0x00	},
	{	0,	"st",		TSTORE,		0x00	},

	/* A only */
	{	0,	"rr",		TAONLY,		0x3E	},
	{	0,	"rrl",		TAONLY,		0x3F	},
	{	0,	"srl",		TAONLY,		0x3D	},

	/*A or EA */
	{	0,	"sl",		TAEA		0x0E0F	},
	{	0,	"sr",		TAEA,		0x3C0C	},

	/* Stack */
	{	0,	"push",		TSTACK,		0x0854	},
	{	0,	"pop",		TSTACK,		0x385C	}
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
	"address required",
	"invalid id",
	"bad mode",
	"constant out of range",
	"data in BSS",
	"segment overflow",
	"segment conflict",
	"divide by zero",
	"autoindexing not permitted",
	"branch out of range",
	"out of range",
	"invalid addressing mode",
	"pointer required"
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
