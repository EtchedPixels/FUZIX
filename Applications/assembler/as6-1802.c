/*
 * 1802 assembler.
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
	{	0,	".commondata",	TSEGMENT,	COMMONDATA },
	{	0,	".buffers",	TSEGMENT,	BUFFERS	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	{	0,	"abs",		TSEGMENT,	ABSOLUTE},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"zp",		TSEGMENT,	ZP	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	"literal",	TSEGMENT,	LITERAL	},
	{	0,	"commondata",	TSEGMENT,	COMMONDATA },
	{	0,	"buffers",	TSEGMENT,	BUFFERS	},
	
	{	0,	"idl",		TIMPL,		0x00	},
	{	0,	"ldn",		TREGNZ,		0x00	},
	{	0,	"inc",		TREG,		0x10	},
	{	0,	"dec",		TREG,		0x20	},
	{	0,	"lda",		TREG,		0x40	},
	{	0,	"str",		TREG,		0x50	},
	{	0,	"irx",		TIMPL,		0x60	},
	{	0,	"out",		TIOPORT,	0x60	},
	{	0,	"inp",		TIOPORT,	0x68	},
	{	0,	"glo",		TREG,		0x80	},
	{	0,	"ghi",		TREG,		0x90	},
	{	0,	"plo",		TREG,		0xA0	},
	{	0,	"phi",		TREG,		0xB0	},
	{	0,	"sep",		TREG,		0xD0	},
	{	0,	"sex", 		TREG,		0xE0	},

	
	/* Implicit stuff */
	{	0,	"ret",		TIMPL,		0x70	},
	{	0,	"dis",		TIMPL,		0x71	},
	{	0,	"ldxa",		TIMPL,		0x72	},
	{	0,	"stxd",		TIMPL,		0x73	},
	{	0,	"adc",		TIMPL,		0x74	},
	{	0,	"sdb",		TIMPL,		0x75	},
	{	0,	"shrc",		TIMPL,		0x76	},
	{	0,	"smb",		TIMPL,		0x77	},
	{	0,	"sav",		TIMPL,		0x78	},
	{	0,	"mark",		TIMPL,		0x79	},
	{	0,	"req",		TIMPL,		0x7A	},
	{	0,	"seq",		TIMPL,		0x7B	},
	{	0,	"shlc",		TIMPL,		0x7E	},
	{	0,	"ldx",		TIMPL,		0xF0	},
	{	0,	"or",		TIMPL,		0xF1	},
	{	0,	"and",		TIMPL,		0xF2	},
	{	0,	"xor",		TIMPL,		0xF3	},
	{	0,	"add",		TIMPL,		0xF4	},
	{	0,	"sd",		TIMPL,		0xF5	},
	{	0,	"shr",		TIMPL,		0xF6	},
	{	0,	"sm",		TIMPL,		0XF7	},
	{	0,	"shl",		TIMPL,		0xFE	},

	/* Immediate 8bit */
	{	0,	"adci",		TIMM8,		0x7C	},
	{	0,	"sbdi",		TIMM8,		0x7D	},
	{	0,	"smbi",		TIMM8,		0x7F	},
	{	0,	"ldi",		TIMM8,		0xF8	},
	{	0,	"ori",		TIMM8,		0xF9	},
	{	0,	"ani",		TIMM8,		0xFA	},
	{	0,	"xri",		TIMM8,		0xFB	},
	{	0,	"adi",		TIMM8,		0xFC	},
	{	0,	"sdi",		TIMM8,		0xFD	},
	{	0,	"smi",		TIMM8,		0xFF	},
	
	/* Branches */
	{	0,	"br",		TREL,		0x30	},
	{	0,	"bq",		TREL,		0x31	},
	{	0,	"bz",		TREL,		0x32	},
	{	0,	"bdf",		TREL,		0x33	},
	{	0,	"b1",		TREL,		0x34	},
	{	0,	"b2",		TREL,		0x35	},
	{	0,	"b3",		TREL,		0x36	},
	{	0,	"b4",		TREL,		0x37	},

	/* skp is branch never rel */
	{	0,	"skp",		TIMPL,		0x38	},
	{	0,	"bnq",		TREL,		0x39	},
	{	0,	"bnz",		TREL,		0x3A	},
	{	0,	"bnf",		TREL,		0x3B	},
	{	0,	"bn1",		TREL,		0x3C	},
	{	0,	"bn2",		TREL,		0x3D	},
	{	0,	"bn3",		TREL,		0x3E	},
	{	0,	"bn4",		TREL,		0x3F	},
	
	/* Long Branches */
	{	0,	"lbr",		TADDR16,		0xC0	},
	{	0,	"lbq",		TADDR16,		0xC1	},
	{	0,	"lbz",		TADDR16,		0xC2	},
	{	0,	"lbdf",		TADDR16,		0xC3	},
	{	0,	"nop",		TNOP,		0xC4	},
	{	0,	"lsnq",		TSKIP,		0xC5	},
	{	0,	"lsnz",		TSKIP,		0xC6	},
	{	0,	"lsnf",		TSKIP,		0xC7	},
	{	0,	"lskp",		TSKIP,		0xC8	},
	{	0,	"lbnq",		TADDR16,		0xC9	},
	{	0,	"lbnz",		TADDR16,		0xCA	},
	{	0,	"lbnf",		TADDR16,		0xCB	},
	{	0,	"lsie",		TSKIP,		0xCC	},
	{	0,	"lsq",		TSKIP,		0xCD	},
	{	0,	"lsz",		TSKIP,		0xCE	},
	{	0,	"lsdf",		TSKIP,		0xCF	},
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
	"branch out of range",		/* 17 */
	"invalid register",		/* 18 */
	"address required",		/* 19 */
	"invalid ID",			/* 20 */
	"invalid I/O",			/* 21 */
	"divide by 0",			/* 22 */
	"constant out of range",	/* 23 */
	"data in BSS",			/* 24 */
	"segment overflow",		/* 25 */
	"data in direct page",		/* 26 */
	"segment conflict",		/* 27 */
	"register may not be 0"		/* 28 */
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
