/*
 * DG Nova assembler.
 *
 * Tables
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"skp",		TCC,		1	},
	{	0,	"szc",		TCC,		2	},
	{	0,	"snc",		TCC,		3	},
	{	0,	"szr",		TCC,		4	},
	{	0,	"snr",		TCC,		5	},
	{	0,	"sez",		TCC,		6	},
	{	0,	"sbn",		TCC,		7	},

	{	0,	"defw",		TDEFW,		XXXX	},
	{	0,	"defs",		TDEFS,		XXXX	},
	{	0,	"defm",		TDEFM,		XXXX	},
	{	0,	"org",		TORG,		XXXX	},
	{	0,	"equ",		TEQU,		XXXX	},
	{	0,	"export",	TEXPORT,	XXXX	},
	{	0,	".word",	TDEFW,		XXXX	},
	{	0,	".blkw",	TDEFS,		XXXX	},
	{	0,	".ascii",	TDEFM,		XXXX	},
	{	0,	".org",		TORG,		XXXX	},
	{	0,	".equ",		TEQU,		XXXX	},
	{	0,	".export",	TEXPORT,	XXXX	},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"zp",		TSEGMENT,	ZP	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	"literal",	TSEGMENT,	LITERAL	},
	{	0,	"commondata",	TSEGMENT,	COMMONDATA },
	{	0,	"buffers",	TSEGMENT,	BUFFERS	},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".zp",		TSEGMENT,	ZP	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	{	0,	".commondata",	TSEGMENT,	COMMONDATA },
	{	0,	".buffers",	TSEGMENT,	BUFFERS	},
	
	{	0,	"jmp",		TMEMORY,	0x0000  },
	{	0,	"jsr",		TMEMORY,	0x0800  },
	{	0,	"isz",		TMEMORY,	0x1000  },
	{	0,	"dsz",		TMEMORY,	0x1800	},
	{	0,	"lda",		TMEMORY,	0x2000	},
	{	0,	"inc",		TMEMORY,	0x3000	},
	{	0,	"sta",		TMEMORY,	0x4000	},

	{	0,	"com",		TALU,		0x8000	},
	{	0,	"neg",		TALU,		0x8100	},
	{	0,	"mov",		TALU,		0x8200	},
	{	0,	"adc",		TALU,		0x8400	},
	{	0,	"sub",		TALU,		0x8500	},
	{	0,	"add",		TALU,		0x8600  },
	{	0,	"and",		TALU,		0x8700	},
	
	{	0,	"dia",		TIO,		0x6100	},
	{	0,	"doa",		TIO,		0x6200	},
	{	0,	"dib",		TIO,		0x6300	},
	{	0,	"dob",		TIO,		0x6400	},
	{	0,	"dic",		TIO,		0x6500	},
	{	0,	"doc",		TIO,		0x6600	},
	{	0,	"skp",		TDEV,		0x6700	},
	
	
	/* Then the post NOVA 1 operations that are less elegant being
	   device ops */
	{	0,	"mtfp",		TAC,		0x6001	},
	{	0,	"mffp",		TAC,		0x6081	},
	{	0,	"mtsp",		TAC,		0x6201	},
	{	0,	"mfsp",		TAC,		0x6281	},
	{	0,	"psha",		TAC,		0x6301	},
	{	0,	"popa",		TAC,		0x6381	},
	{	0,	"sav",		TIMPL,		0x6501	},
	{	0,	"ret",		TIMPL,		0x6581	},
	/* Undocumented SAV n ? */
	
	{	0,	"trap",		TTRAP,		0x8008  },
	{	0,	"ldb",		TBYTE,		0x6101	},
	{	0,	"stb",		TBYTE,		0x6401	},
	
	{	0,	"div",		TIMPL,		0x7641	},
	{	0,	"mul",		TIMPL,		0x76C1	},
	{	0,	"divs",		TIMPL,		0x7E01	},
	{	0,	"muls",		TIMPL,		0x7E81	},	
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
	"relative out of range",
	"skip condition required",
	"invalid register for operation",
	"address required",
	"invalid id",
	"bad addressing mode",
	"divide by 0",
	"constant out of range",
	"data in BSS",
	"segment overflow",
	"data in zero page",
	"bad accumulator",
	"must be zero page or absolute",
	"bad device",
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
	aerr(ADDR_REQUIRED);
}
