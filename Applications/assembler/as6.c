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
	{	0,	"b",		TBR,		B	},
	{	0,	"c",		TBR,		C	},
	{	0,	"d",		TBR,		D	},
	{	0,	"e",		TBR,		E	},
	{	0,	"h",		TBR,		H	},
	{	0,	"l",		TBR,		L	},
	{	0,	"a",		TBR,		A	},
	{	0,	"bc",		TWR,		BC	},
	{	0,	"de",		TWR,		DE	},
	{	0,	"hl",		TWR,		HL	},
	{	0,	"sp",		TWR,		SP	},
	{	0,	"af",		TWR,		AF	},
	{	0,	"af'",		TWR,		AFPRIME	},
	{	0,	"ix",		TWR,		IX	},
	{	0,	"iy",		TWR,		IY	},
	{	0,	"i",		TSR,		I	},
	{	0,	"r",		TSR,		R	},
	{	0,	"nz",		TCC,		CNZ	},
	{	0,	"z",		TCC,		CZ	},
	{	0,	"nc",		TCC,		CNC	},
	{	0,	"po",		TCC,		CPO	},
	{	0,	"pe",		TCC,		CPE	},
	{	0,	"p",		TCC,		CP	},
	{	0,	"m",		TCC,		CM	},
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
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	{	0,	".setcpu",	TSETCPU,	XXXX	},
	{	0,	"nop",		TNOP,		0x0000	},
	{	0,	"rlca",		TNOP,		0x0007	},
	{	0,	"rrca",		TNOP,		0x000F	},
	{	0,	"rla",		TNOP,		0x0017	},
	{	0,	"rra",		TNOP,		0x001F	},
	{	0,	"daa",		TNOP,		0x0027	},
	{	0,	"cpl",		TNOP,		0x002F	},
	{	0,	"scf",		TNOP,		0x0037	},
	{	0,	"ccf",		TNOP,		0x003F	},
	{	0,	"halt",		TNOP,		0x0076	},
	{	0,	"exx",		TNOP,		0x00D9	},
	{	0,	"di",		TNOP,		0x00F3	},
	{	0,	"ei",		TNOP,		0x00FB	},
	{	0,	"neg",		TNOP,		0xED44	},
	{	0,	"retn",		TNOP,		0xED45	},
	{	0,	"reti",		TNOP,		0xED4D	},
	{	0,	"rrd",		TNOP,		0xED67	},
	{	0,	"rld",		TNOP,		0xED6F	},
	{	0,	"ldi",		TNOP,		0xEDA0	},
	{	0,	"cpi",		TNOP,		0xEDA1	},
	{	0,	"ini",		TNOP,		0xEDA2	},
	{	0,	"outi",		TNOP,		0xEDA3	},
	{	0,	"ldd",		TNOP,		0xEDA8	},
	{	0,	"cpd",		TNOP,		0xEDA9	},
	{	0,	"ind",		TNOP,		0xEDAA	},
	{	0,	"outd",		TNOP,		0xEDAB	},
	{	0,	"ldir",		TNOP,		0xEDB0	},
	{	0,	"cpir",		TNOP,		0xEDB1	},
	{	0,	"inir",		TNOP,		0xEDB2	},
	{	0,	"otir",		TNOP,		0xEDB3	},
	{	0,	"lddr",		TNOP,		0xEDB8	},
	{	0,	"cpdr",		TNOP,		0xEDB9	},
	{	0,	"indr",		TNOP,		0xEDBA	},
	{	0,	"otdr",		TNOP,		0xEDBB	},
	{	0,	"rst",		TRST,		XXXX	},
	{	0,	"djnz",		TREL,		0x0010	},
	{	0,	"jr",		TREL,		0x0018	},
	{	0,	"ret",		TRET,		0x00C9	},
	{	0,	"call",		TJMP,		0x00CD	},
	{	0,	"jp",		TJMP,		0x00C3	},
	{	0,	"push",		TPUSH,		0x00C5	},
	{	0,	"pop",		TPUSH,		0x00C1	},
	{	0,	"im",		TIM,		XXXX	},
	{	0,	"in",		TIO,		0x00DB	},
	{	0,	"out",		TIO,		0x00D3	},
	{	0,	"bit",		TBIT,		0xCB40	},
	{	0,	"res",		TBIT,		0xCB80	},
	{	0,	"set",		TBIT,		0xCBC0	},
	{	0,	"rlc",		TSHR,		0xCB00	},
	{	0,	"rrc",		TSHR,		0xCB08	},
	{	0,	"rl",		TSHR,		0xCB10	},
	{	0,	"rr",		TSHR,		0xCB18	},
	{	0,	"sla",		TSHR,		0xCB20	},
	{	0,	"sra",		TSHR,		0xCB28	},
	{	0,	"srl",		TSHR,		0xCB38	},
	{	0,	"inc",		TINC,		0x0004	},
	{	0,	"dec",		TINC,		0x0005	},
	{	0,	"ex",		TEX,		XXXX	},
	{	0,	"add",		TADD,		0x0080	},
	{	0,	"adc",		TADD,		0x0088	},
	{	0,	"sbc",		TADD,		0x0098	},
	{	0,	"sub",		TSUB,		0x0090	},
	{	0,	"and",		TSUB,		0x00A0	},
	{	0,	"xor",		TSUB,		0x00A8	},
	{	0,	"or",		TSUB,		0x00B0	},
	{	0,	"cp",		TSUB,		0x00B8	},
	/* Z180 */
	{	0,	"slp",		TNOP180,	0xED76  },
	{	0,	"otim",		TNOP180,	0xED83	},
	{	0,	"otimr",	TNOP180,	0xED93	},
	{	0,	"otdm",		TNOP180,	0xED8B	},
	{	0,	"otdmr",	TNOP180,	0xED9B	},
	{	0,	"tst",		TTST180,	0xED04	},
	{	0,	"tstio",	TIMMED8,	0xED74	},
	{	0,	"mlt",		TMUL,		0xED4C  },
	{	0,	"in0",		TIO180,		0xED00	},
	{	0,	"out0",		TIO180,		0xED01	},
	/* The joys of ld */
	{	0,	"ld",		TLD,		XXXX    }
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
	"",
	"Z180 instruction",
	"segment conflict",
	"unknown symbol",
	"Too many relative branches"
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
