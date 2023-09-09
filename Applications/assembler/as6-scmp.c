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
	{	0,	"p0",		TBR,		P0	},
	{	0,	"p1",		TBR,		P1	},
	{	0,	"p2",		TBR,		P2	},
	{	0,	"p3",		TBR,		P3	},
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

	/* Memory ref m/ptr/disp format */
	{	0,	"ld",		TMPD,		0xC0	},
	{	0,	"st",		TMPD,		0xC8	},
	{	0,	"and",		TMPD,		0xD0	},
	{	0,	"or",		TMPD,		0xD8	},
	{	0,	"xor",		TMPD,		0xE0	},
	{	0,	"dad",		TMPD,		0xE8	},
	{	0,	"add",		TMPD,		0xF0	},
	{	0,	"cad",		TMPD,		0xF8	},

	/* Memory inc/dec  - ptr/disp */
	{	0,	"ild",		TPD,		0xA8	},
	{	0,	"dld",		TPD,		0xB8	},

	/* Immediate */
	{	0,	"ldi",		TIMM8,		0xC4	},
	{	0,	"ani",		TIMM8,		0xD4	},
	{	0,	"ori",		TIMM8,		0xDC	},
	{	0,	"xri",		TIMM8,		0xE4	},
	{	0,	"dai",		TIMM8,		0xEC	},
	{	0,	"adi",		TIMM8,		0xF4	},
	{	0,	"cai",		TIMM8,		0xFC	},

	/* Transfer */
	{	0,	"jmp",		TREL8,		0x90	},
	{	0,	"jp",		TREL8,		0x94	},
	{	0,	"jz",		TREL8,		0x98	},
	{	0,	"jnz",		TREL8,		0x9C	},

	/* Delay */
	{	0,	"dly",		TIMM8,		0x8F	},

	/* Extension reg */
	{	0,	"lde",		TIMPL,		0x40	},
	{	0,	"xae",		TIMPL,		0x01	},
	{	0,	"ane",		TIMPL,		0x50	},
	{	0,	"ore",		TIMPL,		0x58	},
	{	0,	"xre",		TIMPL,		0x60	},
	{	0,	"dae",		TIMPL,		0x68	},
	{	0,	"ade",		TIMPL,		0x70	},
	{	0,	"cae",		TIMPL,		0x78	},

	/* Pointer moves */
	{	0,	"xpal",		TPTR,		0x30	},
	{	0,	"xpah",		TPTR,		0x34	},
	{	0,	"xppc",		TPTR,		0x3C	},

	/* Shifting */
	{	0,	"sio",		TIMPL,		0x19	},
	{	0,	"sr",		TIMPL,		0x1C	},
	{	0,	"srl",		TIMPL,		0x1D	},
	{	0,	"rr",		TIMPL,		0x1E	},
	{	0,	"rrl",		TIMPL,		0x1F	},

	/* Misc */
	{	0,	"halt",		TIMPL,		0x00	},
	{	0,	"ccl",		TIMPL,		0x02	},
	{	0,	"scl",		TIMPL,		0x03	},
	{	0,	"dint",		TIMPL,		0x04	},
	{	0,	"ien",		TIMPL,		0x05	},
	{	0,	"csa",		TIMPL,		0x06	},
	{	0,	"cas",		TIMPL,		0x07	},
	{	0,	"nop",		TIMPL,		0x08	},

	/* Macro ops */
	{	0,	"js",		TJS,		0x00	}
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
