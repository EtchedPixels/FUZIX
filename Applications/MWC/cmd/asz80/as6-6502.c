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
	{	0,	".ascii",	TDEFM,		XXXX	},
	{	0,	".org",		TORG,		XXXX	},
	{	0,	".equ",		TEQU,		XXXX	},
	{	0,	".export",	TEXPORT,	XXXX	},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"zp",		TSEGMENT,	ZP	},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".zp",		TSEGMENT,	ZP	},
	
	/* Class zero instructions: Those ending 00 */
	{	0,	"bit",		TCLASS0,	0x20	},
	/* JMP (x) is special .. so omit 0x60 entry */
	{	0,	"jmp",		TJMP,		0x40	},
	{	0,	"sty",		TCLASS0,	0x80	},
	{	0,	"ldy",		TCLASS0,	0xA0	},
	{	0,	"cpy",		TCLASS0,	0xC0	},
	{	0,	"cpx",		TCLASS0,	0xE0	},
	

	/* Class one instructions: Those ending 01 */
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
	{	0,	"dec",		TCLASS2,	0xC2	},
	{	0,	"inc",		TCLASS2,	0xE2	},

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
        {	0,	"rts",		TIMPL,		0x60	}
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
	"Bcc out of range",
	"condition required",
	"invalid register for operation",
	"address required",
	"invalid id",
	"bad addressing mode",
	"divide by 0",
	"constant out of range",
	"data in BSS",
	"segment overflow",
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
