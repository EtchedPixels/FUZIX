/*
 * PDP 4/7/9/15
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

	/* Memory ops : upper bits are the opcode */
	{	0,	"cal",		TMEM,		0000000 },
	{	0,	"dac",		TMEM,		0040000 },
	{	0,	"jms",		TMEM,		0100000 },
	{	0,	"dzm",		TMEM,		0140000 },
	{	0,	"lac",		TMEM,		0200000 },
	{	0,	"xor",		TMEM,		0240000 },
	{	0,	"add",		TMEM,		0300000 },
	{	0,	"tad",		TMEM,		0340000 },
	{	0,	"xct",		TMEM,		0400000 },
	{	0,	"isz",		TMEM,		0440000 },
	{	0,	"and",		TMEM,		0500000 },
	{	0,	"sad",		TMEM,		0540000 },
	{	0,	"jmp",		TMEM,		0600000 },
	
	/* OPR instructions: these work in combinations, but have short forms for
	   usual cases */
	{	0,	"opr",		TOPR,		0740000 },
	/* Usual forms */
	{	0,	"nop",		TIMPL,		0740000 },
	{	0,	"cla",		TIMPL,		0750000 },
	{	0,	"cma",		TIMPL,		0740001 },
	{	0,	"cll",		TIMPL,		0744000 },
	{	0,	"cml",		TIMPL,		0740002 },
	{	0,	"ral",		TIMPL,		0740010 },
	{	0,	"rtl",		TIMPL,		0742010 },
	{	0,	"rar",		TIMPL,		0740020 },
	{	0,	"rtr",		TIMPL,		0742020 },
	{	0,	"oas",		TIMPL,		0740004 },
	}	0,	"sma",		TIMPL,		0740100 },
	{	0,	"spa",		TIMPL,		0741100 },
	{	0,	"sza",		TIMPL,		0740200 },
	{	0,	"sna",		TIMPL,		0741200 },
	{	0,	"snl",		TIMPL,		0740400 },
	{	0,	"szl",		TIMPL,		0741400 },
	{	0,	"hlt",		TIMPL,		0740040 },
	
	/* Unique - law: load with a negative word */
	{	0,	"law",		TLAW,		0760000 },
	
	/* iot */
	/* op.4 subdev.2 dev.6 suhdev.2 ac iot3 iot2 iot1 */
	{	0,	"iot",		TIOT,		0700000 },
	{	0,	"csf",		TIMPL,		0700001 },
	{	0,	"iof",		TIMPL,		0700002 },
	{	0,	"cof",		TIMPL,		0700004 },
	{	0,	"ion",		TIMPL,		0700042 },
	{	0,	"con",		TIMPL,		0700044 },
	/* Reader */
	{	0,	"rsf",		TIMPL,		0700101 },
	{	0,	"rcf",		TIMPL,		0700102 },
	{	0,	"rsa",		TIMPL,		0700104 },
	{	0,	"rrb",		TIMPL,		0700112 },
	{	0,	"rsb",		TIMPL,		0700144 },
	/* Punch */
	{	0,	"psf",		TIMPL,		0700201 },
	{	0,	"pcf",		TIMPL,		0700202 },
	{	0,	"pls",		TIMPL,		0700206 },
	/* Teletype */
	{	0,	"ksf",		TIMPL,		0700301 },
	{	0,	"krb",		TIMPL,		0700312 },
	/* Teleprinter */
	{	0,	"tsf",		TIMPL,		0700401 },
	{	0,	"tcf",		TIMPL,		0700402 },
	{	0,	"tls",		TIMPL,		0700406 },
	/* Display 30A */
	{	0,	"dsf",		TIMPL,		0700501 },
	{	0,	"dcf",		TIMPL,		0700502 },
	{	0,	"dls",		TIMPL,		0700506 },
	/* Card punch */
	{	0,	"cpsf",		TIMPL,		0706401 },
	{	0,	"cpcf",		TIMPL,		0706402 },
	{	0,	"cplb",		TIMPL,		0706406 }
	{	0,	"cpse",		TIMPL,		0706442 },
	/* Line Printer - FIXME PDP4 doc is wrong ?*/
	{	0,	"lpsf",		TIMPL,		0706501 },
	{	0,	"lpcf",		TIMPL,		0706502 },
	{	0,	"lpse",		TIMPL,		0706506 },
	{	0,	"lpld",		TIMPL,		0706542 },
	{	0,	"lssf",		TIMPL,		0706506 },	/* 706601 ? */
	{	0,	"lscf",		TIMPL,		0706602 },
	{	0,	"lsls",		TIMPL,		0706606 },
	/* Card Reader */
	{	0,	"crsf",		TIMPL,		0706701 },
	{	0,	"crsa",		TIMPL,		0706704 },
	{	0,	"crrb",		TIMPL,		0706712 },
	{	0,	"crrb",		TIMPL,		0706714 },
	/* Magnetic tape */
	{	0,	"mci",		TIMPL,		0707001 },
	{	0,	"mli",		TIMPL,		0707005 },
	{	0,	"mrs",		TIMPL,		0707012 },
	{	0,	"msc",		TIMPL,		0707101 },
	{	0,	"mwl",		TIMPL,		0707104 },
	{	0,	"mrl",		TIMPL,		0707112 },
	{	0,	"msi",		TIMPL,		0707201 },
	{	0,	"mrm",		TIMPL,		0707202 },
	{	0,	"mwm",		TIMPL,		0707204 },
	{	0,	"nsf",		TIMPL,		0707301 },
	{	0,	"mrr",		TIMPL,		0707302 },
	{	0,	"mwr",		TIMPL,		0707304 },
	
	
	{	0,	"iors",		TIMPL,		0700314 },

	/* PDP 7 : EAE */
	{	0,	"eae",		TEAE,		0640000 },
	/* Common uses */
	{	0,	"lrs",		TIMPLE,		0640500 },
	{	0,	"lls",		TIMPLE,		0640600 },
	{	0,	"als",		TIMPLE,		0640700 },
	{	0,	"lrss",		TIMPLE,		0660500 },
	{	0,	"llss",		TIMPLE,		0660600 },
	{	0,	"alss",		TIMPLE,		0660700 },
	{	0,	"norm",		TIMPLE,		0640444 },
	{	0,	"norms",	TIMPLE,		0660444 },
	{	0,	"mul",		TIMPLE,		0653122 },
	{	0,	"muls", 	TIMPLE,		0657122 },
	{	0,	"div",		TIMPLE,		0640323 },
	{	0,	"divs",		TIMPLE,		0644323 },
	{	0,	"idiv",		TIMPLE,		0653323 },
	{	0,	"idivs",	TIMPLE,		0657323 },
	{	0,	"frdiv",	TIMPLE,		0650323 },
	{	0,	"frdivs",	TIMPLE,		0654323 },
	{	0,	"lacq",		TIMPLE,		0641002 },
	{	0,	"lacs",		TIMPLE,		0641001 },
	{	0,	"clq",		TIMPLE,		0650000 },
	{	0,	"lmq",		TIMPLE,		0652000 },
	{	0,	"abs",		TIMPLE,		0644000 },
	{	0,	"gsm",		TIMPLE,		0664000 },
	{	0,	"osc",		TIMPLE,		0640001 },
	{	0,	"omq",		TIMPLE,		0640002 },
	{	0,	"cmq",		TIMPLE,		0640004 },
	
	/* IOT */
	{	0,	"iton",		TIMPL7,		0700062 },
	{	0,	"coc",		TIMPL7,		0705501 },
	{	0,	"asc",		TIMPL7,		0705502 },
	{	0,	"dsc",		TIMPL7,		0705604 },
	{	0,	"epi",		TIMPL7,		0700004 },
	{	0,	"dpi",		TIMPL7,		0700044 },
	{	0,	"isc",		TIMPL7,		0705504 },
	{	0,	"dbr",		TIMPL7,		0705601 },
	
	/* RTC */
	{	0,	"clsf",		TIMPL7,		0700001 },
	{	0,	"clof",		TIMPL7,		0700004 },
	{	0,	"clon",		TIMPL7,		0700044 },
	
	/* Extend mode */
	{	0,	"sem",		TIMPL7,		0707701 },
	{	0,	"eem",		TIMPL7,		0707702 },
	{	0,	"lem",		TIMPL7,		0707704 },
	{	0,	"emir",		TIMPL7,		0707742 },

	/* Punch */
	{	0,	"psb",		TIMPL7,		0700244 },
	{	0,	"psa",		TIMPL7,		0700204 },

	/* DECtape */
	{	0,	"mmrd",		TIMPL7,		0707512 },
	{	0,	"mmwr",		TIMPL7,		0707504 },
	{	0,	"mmse",		TIMPL7,		0707644 },
	{	0,	"mmlc",		TIMPL7,		0707604 },
	{	0,	"mmrs",		TIMPL7,		0707612 },
	{	0,	"mmdf",		TIMPL7,		0707501 },
	{	0,	"mmbf",		TIMPL7,		0707601 },
	{	0,	"mmef",		TIMPL7,		0707541 },
	
	/* Magtape */
	{	0,	"mscr",		TIMPL7,		0707001 },
	{	0,	"msur",		TIMPL7,		0707101 },
	{	0,	"mccw",		TIMPL7,		0707401 },
	{	0,	"mlca",		TIMPL7,		0707405 },
	{	0,	"mlwc",		TIMPL7,		0707402 },
	{	0,	"mrca",		TIMPL7,		0707414 },
	{	0,	"mdcc",		TIMPL7,		0707042 },
	{	0,	"mctu",		TIMPL7,		0707006 },
	{	0,	"mtcs",		TIMPL7,		0707106 },
	{	0,	"mncm",		TIMPL7,		0707152 },
	{	0,	"mrrc",		TIMPL7,		0707204 },
	{	0,	"mrcr",		TIMPL7,		0707244 },
	{	0,	"msef",		TIMPL7,		0707301 },
	{	0,	"mdef",		TIMPL7,		0707302 },
	{	0,	"mcef",		TIMPL7,		0707322 },
	{	0,	"meef",		TIMPL7,		0707242 },
	{	0,	"mief",		TIMPL7,		0707362 },
	{	0,	"mswf",		TIMPL7,		0707201 },
	{	0,	"mdwf",		TIMPL7,		0707202 },
	{	0,	"mcwf",		TIMPL7,		0707222 },
	{	0,	"mewf",		TIMPL7,		0707242 },
	{	0,	"miwf",		TIMPL7,		0707262 },

	/* Drum */
	{	0,	"drlr",		TIMPL7,		0706006 },
	{	0,	"drlw",		TIMPL7,		0706046 },
	{	0,	"drss",		TIMPL7,		0706106 },
	{	0,	"drcs",		TIMPL7,		0706204 },
	{	0,	"drsf",		TIMPL7,		0706101 },
	{	0,	"drsn",		TIMPL7,		0706201 },
	{	0,	"drcf",		TIMPL7,		0706102 },

	/* PDP 9 */
	
	/* Parity */
	{	0,	"spe",		TIMPL9,		0702701 },
	{	0,	"cpe",		TIMPL9,		0702702 },
	{	0,	"fwp",		TIMPL9,		0702704 },

	/* Memory protect */
	{	0,	"mpsne",	TIMPL9,		0701741 },
	{	0,	"mpsk",		TIMPL9,		0701701 },
	{	0,	"mpeu",		TIMPL9,		0701742 },
	{	0,	"mpcv",		TIMPL9,		0701702 },
	{	0,	"mpcne",	TIMPL9,		0701744 },
	{	0,	"mpld",		TIMPL9,		0701704 },
	
	/* RB09 Disk */
	{	0,	"dscf",		TIMPL9,		0707101 },
	{	0,	"dsld",		TIMPL9,		0707104 },
	{	0,	"dsrd",		TIMPL9,		0707112 },
	{	0,	"dssf",		TIMPL9,		0707121 },
	{	0,	"dslw",		TIMPL9,		0707124 },
	{	0,	"dsrs",		TIMPL9,		0707132 },
	{	0,	"dscs",		TIMPL9,		0707141 },
	{	0,	"dslm",		TIMPL9,		0707142 },
	{	0,	"dsls",		TIMPL9,		0707144 },

	/* API */
	{	0,	"spi",		TIMPL9,		0705501 },
	{	0,	"isa",		TIMPL9,		0705504 },
	{	0,	"dbk",		TIMPL9,		0703304 },
	{	0,	"dbr",		TIMPL9,		0703344 },
	{	0,	"rpl",		TIMPL9,		0705512 },

	/* PDP 15 */
	/* Adds indexing and link and index registers */

	/* The 15 adds swha as an augmented combination */
	{	0,	"swha",		TIMPL15,	0740040 },
	/* Check if these are truely 15 only or jus tonly named in the 15 doc TODO */
	{	0,	"skp",		TIMPL15,	0741000 },
	{	0,	"tca",		TIMPL15,	0740031 },
	{	0,	"stl",		TIMPL15,	0744002 },
	{	0,	"rcl",		TIMPL15,	0744010 },
	{	0,	"rcr",		TIMPL15,	0744020 },
	{	0,	"clc",		TIMPL15,	0750001 },
	{	0,	"las",		TIMPL15,	0750004 },
	{	0,	"glk",		TIMPL15,	0750010 },
	
	/* IOT */
	{	0,	"caf",		TIMPL15,	0703302 },
	{	0,	"spco",		TIMPL15,	0703341 },
	{	0,	"sk15",		TIMPL15,	0707741 },
	{	0,	"sba",		TIMPL15,	0707761 },
	{	0,	"dba",		TIMPL15,	0707764 },
	{	0,	"eba",		TIMPL15,	0707764 },

	/* Index operations 72xx */
	{	0,	"pax",		TIMPL15,	0721000 },
	{	0,	"pal",		TIMPL15,	0722000 },
	{	0,	"pxa",		TIMPL15,	0724000 },
	{	0,	"pxl",		TIMPL15,	0726000 },
	{	0,	"pla",		TIMPL15,	0730000 },
	{	0,	"plx",		TIMPL15,	0731000 },
	/* 9bit signed */
	{	0,	"axs",		T9BIT,		0725000 },
	{	0,	"axr",		T9BIT,		0737000 },
	{	0,	"aac",		T9BIT,		0723000 },
	{	0,	"clx",		TIMPL15,	0735000 },
	{	0,	"cllr",		TIMPL15,	0736000 },
	
	/* IOT for KA15 API extra over the 9 */
	{	0,	"enb",		TIMPL15,	0705521 },
	{	0,	"inh",		TIMPL15,	0705522 },
	{	0,	"res",		TIMPL15,	0707742 },

	/* RP15 */
	{	0,	"dpsf",		TIMPL15,	0706301 },
	{	0,	"dposa",	TIMPL15,	0706302 },
	{	0,	"dprsa",	TIMPL15,	0706312 },
	{	0,	"dpou",		TIMPL15,	0706402 },
	{	0,	"dpru",		TIMPL15,	0706412 },
	{	0,	"dpsa",		TIMPL15,	0706321 },
	{	0,	"dposb",	TIMPL15,	0706322 },
	{	0,	"dprsb",	TIMPL15,	0706332 },
	{	0,	"dplz",		TIMPL15,	0706424 },
	{	0,	"dplo",		TIMPL15,	0706444 },
	{	0,	"dpcn",		TIMPL15,	0706454 },
	{	0,	"dplf",		TIMPL15,	0706464 },
	{	0,	"dpla",		TIMPL15,	0706304 },
	{	0,	"dpca",		TIMPL15,	0706344 },
	{	0,	"dpwc",		TIMPL15,	0706364 },
	{	0,	"dpoa",		TIMPL15,	0706422 },
	{	0,	"dpra",		TIMPL15,	0706432 },
	{	0,	"dpoc",		TIMPL15,	0706442 },
	{	0,	"dprc",		TIMPL15,	0706452 },
	{	0,	"dpow",		TIMPL15,	0706462 },
	{	0,	"dprw",		TIMPL15,	0706472 },
	{	0,	"dpcs",		TIMPL15,	0706324 },
	{	0,	"dpcf",		TIMPL15,	0706404 },
	{	0,	"dpsj",		TIMPL15,	0706341 },
	{	0,	"dpse",		TIMPL15,	0706361 },
	{	0,	"dpom",		TIMPL15,	0706342 },
	{	0,	"dprm",		TIMPL15,	0706352 },
	{	0,	"dpem",		TIMPL15,	0706401 },
	{	0,	"dplm",		TIMPL15,	0706411 }

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
