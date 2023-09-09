/*
 * Bytecode assembler.
 *
 * Tables
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"defb",		TDEFB,		XXXX	},
	{	0,	"defw",		TDEFW,		XXXX	},
	{	0,	"defs",		TDEFS,		XXXX	},
	{	0,	"db",		TDEFB,		XXXX	},
	{	0,	"dw",		TDEFW,		XXXX	},
	{	0,	"ds",		TDEFS,		XXXX	},
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
	{	0,	".abs",		TSEGMENT,	ABSOLUTE},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".zp",		TSEGMENT,	ZP	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	

	/* Operations are in four classes
		- no data
		- byte data
		- word data
		- long data (4 byte) */
			
	{	0,	"shl",		TIMPL,		0,	},
	{	0,	"shll",		TIMPL,		3,	},
	{	0,	"shru",		TIMPL,		6,	},
	{	0,	"shrul",	TIMPL,		9,	},
	{	0,	"shr",		TIMPL,		12,	},
	{	0,	"shrl",		TIMPL,		15,	},
	{	0,	"cceq",		TIMPL,		18,	},
	{	0,	"cceql",	TIMPL,		21,	},
	{	0,	"band",		TIMPL,		24,	},
	{	0,	"bandc",	TIMPL,		24,	},
	{	0,	"bandl",	TIMPL,		27,	},
	{	0,	"mul",		TIMPL,		30,	},
	{	0,	"mull",		TIMPL,		33,	},
	{	0,	"divu",		TIMPL,		36,	},
	{	0,	"divul",	TIMPL,		39,	},
	{	0,	"div",		TIMPL,		42,	},
	{	0,	"divl",		TIMPL,		45,	},
	{	0,	"remu",		TIMPL,		48,	},
	{	0,	"remul",	TIMPL,		51,	},
	{	0,	"rem",		TIMPL,		54,	},
	{	0,	"reml",		TIMPL,		57,	},
	{	0,	"plus",		TIMPL,		60,	},
	{	0,	"plusc",	TIMPL,		60,	},
	{	0,	"plusl",	TIMPL,		63,	},
	{	0,	"minus",	TIMPL,		66,	},
	{	0,	"minusc",	TIMPL,		66,	},
	{	0,	"minusl",	TIMPL,		69,	},
	{	0,	"xor",		TIMPL,		72,	},
	{	0,	"xorc",		TIMPL,		72,	},
	{	0,	"xorl",		TIMPL,		75,	},
	{	0,	"ccltu",	TIMPL,		78,	},
	{	0,	"ccltul",	TIMPL,		81,	},
	{	0,	"cclt",		TIMPL,		84,	},
	{	0,	"ccltl",	TIMPL,		87,	},
	{	0,	"ccgtu",	TIMPL,		90,	},
	{	0,	"ccgtul",	TIMPL,		93,	},
	{	0,	"ccgt",		TIMPL,		96,	},
	{	0,	"ccgtl",	TIMPL,		99,	},
	{	0,	"or",		TIMPL,		102,	},
	{	0,	"orc",		TIMPL,		102,	},
	{	0,	"orl",		TIMPL,		105,	},
	{	0,	"not",		TIMPL,		108,	},
	{	0,	"notc",		TIMPL,		108,	},
	{	0,	"notl",		TIMPL,		111,	},
	{	0,	"assignc",	TIMPL,		114,	},
	{	0,	"assign",	TIMPL,		117,	},
	{	0,	"assignl",	TIMPL,		120,	},
	{	0,	"derefc",	TIMPL,		123,	},
	{	0,	"derefuc",	TIMPL,		126,	},
	{	0,	"deref",	TIMPL,		129,	},
	{	0,	"derefu",	TIMPL,		129,	},
	{	0,	"derefl",	TIMPL,		132,	},
	{	0,	"dereful",	TIMPL,		132,	},
	{	0,	"negate",	TIMPL,		135,	},
	{	0,	"negatel",	TIMPL,		138,	},
	{	0,	"callfunc",	TIMPL,		141,	},
	{	0,	"sex",		TIMPL,		144,	},
	{	0,	"sexl",		TIMPL,		147,	},
	{	0,	"constuc",	TBYTE,		150,	},
	{	0,	"constc",	TBYTE,		153,	},
	{	0,	"constu",	TWORD,		156,	},
	{	0,	"const",	TWORD,		156,	},
	{	0,	"constul",	TLONG,		159,	},
	{	0,	"constl",	TLONG,		159,	},
	{	0,	"pop",		TIMPL,		162,	},
	{	0,	"popc",		TIMPL,		162,	},
	{	0,	"popl",		TIMPL,		165,	},
	{	0,	"pushc",	TIMPL,		168,	},
	{	0,	"push",		TIMPL,		168,	},
	{	0,	"pushl",	TIMPL,		171,	},
	{	0,	"bool",		TIMPL,		174,	},
	{	0,	"boolc",	TIMPL,		174,	},
	{	0,	"booll",	TIMPL,		177,	},
	{	0,	"loadl",	TWORD,		180,	},
	{	0,	"spmod",	TWORD,		183,	},
	{	0,	"exit",		TIMPL,		186,	},
	{	0,	"jump",		TWORD,		189,	},
	{	0,	"jfalse",	TWORD,		192,	},
	{	0,	"jtrue",	TWORD,		195,	},
	{	0,	"switchc",	TWORD,		198,	},
	{	0,	"switch",	TWORD,		201,	},
	{	0,	"switchl",	TWORD,		204,	},
	{	0,	"spmod8",	TBYTE,		207,	},
	{	0,	"loadl8",	TBYTE,		210,	},
	{	0,	"constl",	TLONG,		213,	},
	{	0,	"call",		TWORD,		216,	},
	{	0,	"derefoc",	TBYTE,		219,	},
	{	0,	"derefouc",	TBYTE,		222,	},
	{	0,	"derefo",	TBYTE,		225,	},
	{	0,	"derefou",	TBYTE,		225,	},
	{	0,	"derefol",	TBYTE,		228,	},
	{	0,	"derefoul",	TBYTE,		228,	},
	{	0,	"getsp",	TIMPL,		231,	},
	{	0,	"setsp",	TIMPL,		234,	},
	{	0,	"cpush",	TWORD,		237,	},
	{	0,	"byteu",	TIMPL,		240,	},
	{	0,	"dup",		TIMPL,		243,	},
	{	0,	"swap",		TIMPL,		246,	},

	/* We default ton 8080 call instruction and offset header */
	{	0,	"z80call",	TWORD,		205	}

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
	"address required",		/* 17 */
	"invalid ID",			/* 18 */
	"divide by 0",			/* 19 */
	"constant out of range",	/* 20 */
	"data in BSS",			/* 21 */
	"segment overflow",		/* 22 */
	"data in direct page",		/* 23 */
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
