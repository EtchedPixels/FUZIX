/*
 * 8080 and 8085
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
	{	0,	"b",		TBR,		B	},
	{	0,	"c",		TBR,		C	},
	{	0,	"d",		TBR,		D	},
	{	0,	"e",		TBR,		E	},
	{	0,	"h",		TBR,		H	},
	{	0,	"l",		TBR,		L	},
	{	0,	"a",		TBR,		A	},
	{	0,	"m",		TBR,		M	},
	{	0,	"sp",		TWR,		SP	},
	{	0,	"psw",		TWR,		PSW	},
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
	{	0,	".setcpu",	TSETCPU,	XXXX	},
	/* First quadrant, various miscellaneous groupings with 3 bit
	   register encoding */
	{	0,	"nop",		TIMPL,		0x0000	},
	{	0,	"lxi",		TREG16_I16,	0x0001  },
	{	0,	"stax",		TREG16BD,	0x0002  },
	{	0,	"inx",		TREG16,		0x0003  },
	{	0,	"inr",		TREG8H,		0x0004  },
	{	0,	"dcr",		TREG8H,		0x0005  },
	{	0,	"mvi",		TREG8_I8,	0x0006  },
	{	0,	"rlc",		TIMPL,		0x0007  },
	{	0,	"dsub",		TIMPL85,	0x0008  },
	{	0,	"dad",		TREG16,		0x0009  },
	{	0,	"ldax",		TREG16BD,	0x000A  },
	{	0,	"dcx",		TREG16,		0x000B  },
	{	0,	"rrc",		TIMPL,		0x000F  },
	{	0,	"arhl",		TIMPL85,	0x0010  },
	{	0,	"ral",		TIMPL,		0x0017  },
	{	0,	"rdel",		TIMPL85,	0x0018  },
	{	0,	"rar",		TIMPL,		0x001F  },
	{	0,	"rim",		TIMPL85,	0x0020  },
	{	0,	"shld",		TI16,		0x0022  },
	{	0,	"daa",		TIMPL,		0x0027  },
	{	0,	"ldhi",		TI8_85,		0x0028  },
	{	0,	"lhld",		TI16,		0x002A  },
	{	0,	"cma",		TIMPL,		0x002F  },
	{	0,	"sim",		TIMPL85,	0x0030  },
	{	0,	"sta",		TI16,		0x0032	},
	{	0,	"stc",		TIMPL,		0x0037  },
	{	0,	"ldsi",		TI8_85,		0x0038  },
	{	0,	"lda",		TI16,		0x003A	},
	{	0,	"cmc",		TIMPL,		0x003F  },
	/* Second quadrant, 2 bits 01 and a pair of 3 bit register encodes */
	{	0,	"mov",		TMOV,		0x0040  },
	/* Special case mov m,m is in fact hlt */
	{	0,	"hlt",		TIMPL,		0x0076  },
	/* Third quadrant,  2 bits 10, 3 bits operation, 3 bit register encode */
	{	0,	"add",		TREG8,		0x0080  },
	{	0,	"adc",		TREG8,		0x0088  },
	{	0,	"sub",		TREG8,		0x0090  },
	{	0,	"sbb",		TREG8,		0x0098  },
	{	0,	"ana",		TREG8,		0x00A0  },
	{	0,	"xra",		TREG8,		0x00A8  },
	{	0,	"ora",		TREG8,		0x00B0  },
	{	0,	"cmp",		TREG8,		0x00B8  },
	/* Fourth quadrant - bit of a mix */
	{	0,	"rnz",		TIMPL,		0x00C0  },
	{	0,	"pop",		TREG16_P,	0x00C1  },
	{	0,	"jnz",		TI16,		0x00C2  },
	{	0,	"jmp",		TI16,		0x00C3  },
	{	0,	"cnz",		TI16,		0x00C4  },
	{	0,	"push",		TREG16_P,	0x00C5  },
	{	0,	"adi",		TI8,		0x00C6  },
	{	0,	"rst",		TRST,		0x00C7  },
	{	0,	"rz",		TIMPL,		0x00C8  },
	{	0,	"ret",		TIMPL,		0x00C9  },
	{	0,	"jz",		TI16,		0x00CA  },
	{	0,	"rstv",		TIMPL85,	0x00CB	},
	{	0,	"cz",		TI16,		0x00CC  },
	{	0,	"call",		TI16,		0x00CD  },
	{	0,	"aci",		TI8,		0x00CE  },
	{	0,	"rnc",		TIMPL,		0x00D0  },
	{	0,	"jnc",		TI16,		0x00D2  },
	{	0,	"out",		TI8,		0x00D3  },
	{	0,	"cnc",		TI16,		0x00D4  },
	{	0,	"sui",		TI8,		0x00D6  },
	{	0,	"rc",		TIMPL,		0x00D8  },
	{	0,	"shlx",		TIMPL85,	0x00D9  },
	{	0,	"jc",		TI16,		0x00DA  },
	{	0,	"in",		TI8,		0x00DB  },
	{	0,	"cc",		TI16,		0x00DC  },
	{	0,	"jnk",		TI16_85,	0x00DD  },
	{	0,	"sbi",		TI8,		0x00DE  },
	{	0,	"rpo",		TIMPL,		0x00E0  },
	{	0,	"jpo",		TI16,		0x00E2  },
	{	0,	"xthl",		TIMPL,		0x00E3	},
	{	0,	"cpo",		TI16,		0x00E4  },
	{	0,	"ani",		TI8,		0x00E6  },
	{	0,	"rpe",		TIMPL,		0x00E8  },
	{	0,	"pchl",		TIMPL,		0x00E9  },
	{	0,	"jpe",		TI16,		0x00EA  },
	{	0,	"xchg",		TIMPL,		0x00EB  },
	{	0,	"cpe",		TI16,		0x00EC  },
	{	0,	"lhlx",		TIMPL85,	0x00ED  },
	{	0,	"xri",		TI8,		0x00EE  },
	{	0,	"rp",		TIMPL,		0x00F0  },
	{	0,	"jp",		TI16,		0x00F2  },
	{	0,	"di",		TIMPL,		0x00F3  },
	{	0,	"cp",		TI16,		0x00F4  },
	{	0,	"ori",		TI8,		0x00F6  },
	{	0,	"rm",		TIMPL,		0x00F8  },
	{	0,	"sphl",		TIMPL,		0x00F9  },
	{	0,	"jm",		TI16,		0x00FA  },
	{	0,	"ei",		TIMPL,		0x00FB  },
	{	0,	"cm",		TI16,		0x00FC  },
	{	0,	"jk",		TI16,		0x00FD  },
	{	0,	"cpi",		TI8,		0x00FE  },
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
	"",
	"condition required",
	"invalid register for operation",
	"address required",
	"invalid id",
	"",
	"divide by 0",
	"constant out of range",
	"data in BSS",
	"segment overflow",
	"",
	"8085 instruction",
	"segment conflict",
	"unknown symbol"
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
