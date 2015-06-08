/* globvar.h - global variables for assembler */

/* global control and bookkeeping */

EXTERN bool_t binaryc;		/* current binary code flag */
EXTERN bool_t binaryg;		/* global binary code flag */
EXTERN offset_t binmbuf;	/* offset in binary code buffer for memory */
EXTERN bool_t binmbuf_set;	/* set to 1 when binmbuf set by org */

EXTERN unsigned char dirpag;	/* direct page */

EXTERN bool_t globals_only_in_obj;	/* global symbols only in object file */

EXTERN bool_t jumps_long;	/* make all jumps long */

EXTERN unsigned char mapnum;	/* global map number */

EXTERN bool_t objectc;		/* current object code flag */
EXTERN bool_t objectg;		/* global object code flag */

EXTERN bool_t pass;		/* pass, FALSE means 0, TRUE means 1 */

EXTERN offset_t progent;	/* program entry point */

EXTERN bool_t symgen;		/* generate symbol table flag */

EXTERN unsigned toterr;		/* total errors */
EXTERN unsigned totwarn;	/* total warnings */

EXTERN bool_t list_force;	/* Force line to be listed - no error */

/* bookeeping for current line */

EXTERN char *linebuf;		/* buffer */

/* for symbol table routines */

EXTERN unsigned char inidata;	/* init sym entry data governed by "u" flag */
EXTERN struct sym_s **spt;	/* symbol pointer table */
EXTERN struct sym_s **spt_top;	/* top of symbol ptr table */

/* for translator */

EXTERN struct sym_s *label;	/* non-null if valid label starts line */
EXTERN unsigned char pedata;	/* shows how PROGENT bound, flags like LCDATA*/
EXTERN unsigned char popflags;	/* pseudo-op flags */

/* for BLOCK stack */

EXTERN struct block_s *blockstak;	/* stack ptr */
EXTERN unsigned char blocklevel;	/* nesting level */

/* for IF stack */

EXTERN struct if_s *ifstak;	/* stack ptr */
EXTERN unsigned char iflevel;	/* nesting level */
EXTERN bool_t ifflag;		/* set if assembling */

/* location counters for various segments */

EXTERN offset_t lc;		/* location counter */
EXTERN unsigned char lcdata;	/* shows how lc is bound */
				/* FORBIT is set if lc is forward referenced */
				/* RELBIT is is if lc is relocat. (not ASEG) */
EXTERN offset_t lcjump; 	/* lc jump between lines */

EXTERN offset_t oldlabel; 	/* Used for checking for moving labels */
#ifdef LOW_BYTE
#define mcount (((unsigned char *) &lcjump)[LOW_BYTE])
				/* low byte of lcjump */
#else
#define mcount lcjump		/* I think this is just a speed hack */
#endif
EXTERN struct lc_s *lcptr;	/* top of current spot in lctab */
EXTERN struct lc_s *lctab;	/* start of lctab */
EXTERN struct lc_s *lctabtop;	/* top of lctab */

/* for code generator */

EXTERN opsize_t mnsize;		/* 1 if forced byte operand size, else 0 */
EXTERN opcode_t page;
EXTERN opcode_t opcode;
EXTERN opcode_t postb;		/* postbyte, 0 if none */
EXTERN unsigned char pcrflag;	/* OBJ_RMASK set if addressing is PC-relative */
EXTERN int last_pass;		/* Pass number of last pass */
EXTERN int dirty_pass;		/* Set if this pass had a label movement */

EXTERN int textseg;		/* Text segment id */

#ifdef I80386

EXTERN opcode_t aprefix;	/* address size prefix or 0 */
EXTERN bool_t asld_compatible;	/* asld compatibility flag */
EXTERN opsize_t defsize;	/* current default size */
EXTERN opsize_t idefsize;	/* initial default size */
EXTERN opcode_t oprefix;	/* operand size prefix or 0 */
EXTERN opcode_t sprefix;	/* segment prefix or 0 */
EXTERN opcode_t sib;		/* scale-index-base byte */

EXTERN int cpuid;		/* Assembler instruction limit flag */
EXTERN int origcpuid;		/* Assembler instruction limit flag */

#endif

/* miscellaneous */

extern char hexdigit[];

/* cpuid functions */
#ifdef I80386
#ifndef __AS386_16__
#define iscpu(x) (cpuid>=(x))
#define needcpu(x) do{ if(cpuid<(x)) {warning(CPUCLASH); cpuid|=0x10;} }while(0)
#define setcpu(x) (cpuid=(x))
#define cpuwarn() (cpuid&=0xF)
#endif
#endif

#ifndef setcpu
#define needcpu(x)
#define setcpu(x)
#define cpuwarn()
#endif

