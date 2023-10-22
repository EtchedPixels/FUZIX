/*
 * 6502 assembler.
 *
 * Tables
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"a",		TBR,		A	},
	{	0,	"x",		TBR,		X	},
	{	0,	"y",		TBR,		Y	},

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
	{	0,	".6502",	TCPU,		CPU_6502	},
	{	0,	".65c02",	TCPU,		CPU_65C02	},
	{	0,	".65c816",	TCPU,		CPU_65C816	},
	{	0,	".i8",		TI,		1	},
	{	0,	".i16",		TI,		2	},
	{	0,	".a8",		TA,		1	},
	{	0,	".a16",		TA,		2	},
	
	/* Class zero instructions: Those ending 00 */
	/* X form is thode that grow imm, ab inx,x and dp index x on
	  65C02/65C816 */
	{	0,	"bit",		TCLASS0X,	0x20	},
	/* JMP (x) is special .. so omit 0x60 entry */
	{	0,	"jmp",		TJMP,		0x40	},
	{	0,	"sty",		TCLASS0,	0x80	},
	{	0,	"ldy",		TCLASS0,	0xA0	},
	{	0,	"cpy",		TCLASS0,	0xC0	},
	{	0,	"cpx",		TCLASS0,	0xE0	},
	

	/* Class one instructions: Those ending 01 */
	/* Most get dp indirect on C02, long, sr and sr indexed on 816 */
	{	0,	"ora",		TCLASS1,	0x01	},
	{	0,	"and",		TCLASS1,	0x21	},
	{	0,	"eor",		TCLASS1,	0x41	},
	{	0,	"adc",		TCLASS1,	0x61	},
	{	0,	"sta",		TCLASS1,	0x81	},
	{	0,	"lda",		TCLASS1,	0xA1	},
	{	0,	"cmp",		TCLASS1,	0xC1	},
	{	0,	"sbc",		TCLASS1,	0xE1	},
	
	/* Class two instructions: Those ending 10 */
	{	0,	"asl",		TCLASS2,	0x02	},
	{	0,	"rol",		TCLASS2,	0x22	},
	{	0,	"lsr",		TCLASS2,	0x42	},
	{	0,	"ror",		TCLASS2,	0x62	},
	{	0,	"stx",		TCLASS2Y,	0x82	},
	{	0,	"ldx",		TCLASS2Y,	0xA2	},
	/* with a special form of dec a on C02 and later */
	{	0,	"dec",		TCLASS2A,	0x3AC2	},
	{	0,	"inc",		TCLASS2A,	0x1AE2	},

	/* Bcc instructions */
        {	0,	"bpl",		TREL8,		0x10	},
        {	0,	"bmi",		TREL8,		0x30	},
        {	0,	"bvc",		TREL8,		0x50	},
        {	0,	"bvs",		TREL8,		0x70	},
        {	0,	"bcc",		TREL8,		0x90	},
        {	0,	"bcs",		TREL8,		0xB0	},
        {	0,	"bne",		TREL8,		0xD0	},
        {	0,	"beq",		TREL8,		0xF0	},

        /* x8 instructions */        
        {	0,	"php",		TIMPL,		0x08	},
        {	0,	"clc",		TIMPL,		0x18	},
        {	0,	"plp",		TIMPL,		0x28	},
        {	0,	"sec",		TIMPL,		0x38	},
        {	0,	"pha",		TIMPL,		0x48	},
        {	0,	"cli",		TIMPL,		0x58	},
        {	0,	"pla",		TIMPL,		0x68	},
        {	0,	"sei",		TIMPL,		0x78	},
        {	0,	"dey",		TIMPL,		0x88	},
        {	0,	"tya",		TIMPL,		0x98	},
        {	0,	"tay",		TIMPL,		0xA8	},
        {	0,	"clv",		TIMPL,		0xB8	},
        {	0,	"iny",		TIMPL,		0xC8	},
        {	0,	"cld",		TIMPL,		0xD8	},
        {	0,	"inx",		TIMPL,		0xE8	},
        {	0,	"sed",		TIMPL,		0xF8	},
        
        /* xA instructions */
        {	0,	"txa",		TIMPL,		0x8A	},
        {	0,	"txs",		TIMPL,		0x9A	},
        {	0,	"tax",		TIMPL,		0xAA	},
        {	0,	"tsx",		TIMPL,		0xBA	},
        {	0,	"dex",		TIMPL,		0xCA	},
        {	0,	"nop",		TIMPL,		0xEA	},
        
        /* Weird stuff */
        {	0,	"brk",		TBRK,		0x00	},
        {	0,	"jsr",		TJSR,		0x20	},
        {	0,	"rti",		TIMPL,		0x40	},
        {	0,	"rts",		TIMPL,		0x60	},

        /* 65C02 - also adds Zero Page indirect and absolute indexed
           indirect */
        {	0,	"bra",		TREL8C,		0x80	},
        {	0,	"phx",		TIMPLC,		0xDA	},
        {	0,	"phy",		TIMPLC,		0x5A	},
        {	0,	"plx",		TIMPLC,		0xFA	},
        {	0,	"ply",		TIMPLC,		0x7A	},
        /* Weird stuffed in extra instruction in four formats */
        {	0,	"stz",		TSTZ,		0x00	},
        {	0,	"trb",		TABDP,		0x1C14	},
        {	0,	"tsb",		TABDP,		0x0C04	},

        /* 65C816 - 16bit modes, rep/sep and direct page
           adds 24bit addressing and indexing */
        {	0,	"brl",		TREL16,		0x82	},
        {	0,	"cop",		TBRK,		0x02	},
        {	0,	"jml",		TJML,		0x5C	},
        {	0,	"jsl",		TLONG,		0x22	},
        {	0,	"mvn",		TMVN,		0x54	},
        {	0,	"mvp",		TMVN,		0x44	},
        {	0,	"pea",		TIMM16,		0xF4	},
        {	0,	"pei",		TPEI,		0xD4	},
        {	0,	"per",		TREL16,		0x62	},
        {	0,	"phb",		TIMPL16,	0x8B	},
        {	0,	"phd",		TIMPL16,	0x0B	},
        {	0,	"phk",		TIMPL16,	0x4B	},
        {	0,	"plb",		TIMPL16,	0xAB	},
        {	0,	"pld",		TIMPL16,	0x2B	},
        {	0,	"rep",		TREP,		0xC2	},
        {	0,	"rtl",		TIMPL16,	0x40	},
        {	0,	"sep",		TREP,		0xE2	},
        {	0,	"stp",		TIMPL16,	0xDB	},
        {	0,	"tad",		TIMPL16,	0x5B	},
        {	0,	"tcd",		TIMPL16,	0x5B	},
        {	0,	"tas",		TIMPL16,	0x1B	},
        {	0,	"tcs",		TIMPL16,	0x1B	},
        {	0,	"tdc",		TIMPL16,	0x7B	},
        {	0,	"tsc",		TIMPL16,	0x3B	},
        {	0,	"txy",		TIMPL16,	0x9B	},
        {	0,	"tyx",		TIMPL16,	0x98	},
        {	0,	"wait",		TIMPL16,	0xCB	},
        {	0,	"wdm",		TIMPL16,	0x42	},
        {	0,	"xba",		TIMPL16,	0xEB	},
        {	0,	"swa",		TIMPL16,	0xEB	},
        {	0,	"xce",		TIMPL16,	0xFB	}
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
	"phase error",
	"multiple definitions",
	"syntax error",
	"must be absolute",
	"missing delimiter",
	"invalid constant",
	"Bcc out of range",
	"condition required",
	"invalid register for operation",
	"address required",		/* 20 */
	"invalid id",
	"bad addressing mode",
	"divide by 0",
	"constant out of range",
	"data in BSS",
	"segment overflow",
	"segment conflict",
	"data in zero page",
	"out of range",
	"unsupported by this cpu",
	"too many resizable branches"
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
