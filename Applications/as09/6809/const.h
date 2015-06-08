#define align(x)		/* ((x) = ((int) (x) + (4-1)) & ~(4-1)) */
#define LOW_BYTE 0		/* must be changed for big-endian */

/* const.h - constants for assembler */

/* major switches */

#undef  I80386			/* generate 80386 code */
#define MC6809			/* generate 6809 code */
#define MNSIZE			/* allow byte size in mnemonic, e.g. "movb" */
#undef  SOS_EDOS		/* source OS is EDOS */

/* defaults */

#define DIRCHAR '/'		/* character separating filename from dir */
#define INBUFSIZE 8192
#define SOS_EOLSTR "\012"

/* defaults modified by switches */

#ifdef SOS_EDOS
# undef INBUFSIZE
# define INBUFSIZE 512
# undef SOS_EOLSTR
# define SOS_EOLSTR "\015\012"
# define STAKSIZ 256		/* table grows up to stack less this */
#endif

/* booleans */

#define FALSE 0
#define TRUE 1

/* ASCII constants */

#define ETB  23

/* C tricks */

#define EXTERN extern
#define FORWARD static
#define PRIVATE static
#define PUBLIC
#define NULL 0

/* O/S constants */

#define CREAT_PERMS 0666
#define EOF (-1)
#define STDIN 0
#define STDOUT 1

/* register codes (internal to assembler) */

#ifdef I80386

/* index regs must be first */

#define BPREG 0
#define BXREG 1
#define DIREG 2
#define SIREG 3
#define MAX16BITINDREG 3

#define EAXREG 4
#define EBPREG 5
#define EBXREG 6
#define ECXREG 7
#define EDIREG 8
#define EDXREG 9
#define ESIREG 10
#define ESPREG 11
#define MAXINDREG 11

#define AXREG 12
#define CXREG 13
#define DXREG 14
#define SPREG 15

#define AHREG 16
#define ALREG 17
#define BHREG 18
#define BLREG 19
#define CHREG 20
#define CLREG 21
#define DHREG 22
#define DLREG 23

#define CSREG 24
#define DSREG 25
#define ESREG 26
#define FSREG 27
#define GSREG 28
#define SSREG 29

#define CR0REG 30
#define CR2REG 31
#define CR3REG 32
#define DR0REG 33
#define DR1REG 34
#define DR2REG 35
#define DR3REG 36
#define DR6REG 37
#define DR7REG 38
#define TR6REG 39
#define TR7REG 40

#define NOREG 41

#endif				/* I80386 */

#ifdef MC6809

/* index regs must be first, then PC, then other regs */

#define AREG  5
#define BREG  6
#define CCREG 7
#define DPREG 8
#define DREG  9
#define MAXINDREG 3
#define NOREG 10
#define PCREG 4
#define SREG  0
#define UREG  1
#define XREG  2
#define YREG  3

#endif

#ifdef I80386

/* type and size keywords */

#define BYTEOP     0
#define DWORDOP    1
#define FWORDOP    2
#define FAROP      3
#define PTROP      4
#define PWORDOP    5
#define QWORDOP    6
#define TBYTEOP    7
#define WORDOP     8
#endif

/* special chars */

#define EOL		0
#define MACROCHAR	'?'

/* symbol codes */

/* the first 2 must be from chars in identifiers */
#define IDENT		0
#define INTCONST	1

/* the next few are best for other possibly-multi-char tokens */
#define ADDOP		2	/* also ++ */
#define BINCONST	3
#define CHARCONST	4
#define GREATERTHAN	5	/* also >> and context-sensitive */
#define HEXCONST	6
#define LESSTHAN	7	/* also << and context-sensitive */
#define SUBOP		8	/* also -- */
#define WHITESPACE	9

#define ANDOP		10
#define COMMA		11
#define EOLSYM		12
#define EQOP		13
#define IMMEDIATE	14
#define INDIRECT	15
#define LBRACKET	16
#define LPAREN		17
#define MACROARG	18
#define NOTOP		19
#define OROP		20
#define OTHERSYM	21
#define POSTINCOP	22
#define PREDECOP	23
#define RBRACKET	24
#define RPAREN		25
#define SLASH		26	/* context-sensitive */
#define SLOP		27
#define SROP		28
#define STAR		29	/* context-sensitive */
#define STRINGCONST	30
#define COLON		31

/* these are from assembler errors module */

/* syntax errors */

#define COMEXP 0
#define DELEXP 1
#define FACEXP 2
#define IREGEXP 3
#define LABEXP 4
#define LPEXP 5
#define OPEXP 6
#define RBEXP 7
#define REGEXP 8
#define RPEXP 9
#define SPEXP 10

/* expression errors */

#define ABSREQ 11
#define NONIMPREQ 12
#define RELBAD 13

/* label errors */

#define ILLAB 14
#define MACUID 15
#define MISLAB 16
#define MNUID 17
#define REGUID 18
#define RELAB 19
#define UNBLAB 20
#define UNLAB 21
#define VARLAB 22

/* addressing errors */

#define ABOUNDS 23
#define DBOUNDS 24
#define ILLMOD 25
#define ILLREG 26

/* control structure errors */

#define ELSEBAD 27
#define ELSEIFBAD 27
#define ENDBBAD 28
#define ENDIFBAD 27
#define EOFBLOCK 29
#define EOFIF 30

#define EOFLC 31
#define EOFMAC 32
#define FAILERR 33

/* overflow errors */

#define BLOCKOV 34
#define BWRAP 35
#define COUNTOV 36
#define COUNTUN 37
#define GETOV 38
#define IFOV 39

#define LINLONG 40
#define MACOV 41
#define OBJSYMOV 42
#define OWRITE 43
#define PAROV 44
#define SYMOV 45
#define SYMOUTOV 46

/* i/o errors */

#define OBJOUT 47

/* miscellaneous errors */

#define CTLINS 48
#define FURTHER 49
#define NOIMPORT 50
#define NOTIMPLEMENTED 51
#define REENTER 52
#define SEGREL 53

/* warnings */

#define MINWARN 54
#define ALREADY 54
#define SHORTB 55

/* symbol table entry */

				/* type entry contains following flags */
#define ENTBIT   (1<<0)		/* entry point (=OBJ_N_MASK) */
#define COMMBIT  (1<<1)		/* common */
#define LABIT    (1<<2)		/* label (a PC location or defined by EQU) */
#define MNREGBIT (1<<3)		/* mnemonic for op or pseudo-op, or register */
#define MACBIT   (1<<4)		/* macro */
#define REDBIT   (1<<5)		/* redefined */
#define VARBIT   (1<<6)		/* variable (i.e. something defined by SET) */
#define EXPBIT   (1<<7)		/* exported (= OBJ_E_MASK) */

				/* data entry contains following flags, valid */
				/* for expressions as well as syms */
#define PAGE1    (1<<0)		/* page 1 machine op = MNREGBIT \ PAGE1 */
#define PAGE2    (1<<1)		/* page 2 machine op = MNREGBIT \ PAGE2 */
#define REGBIT   (1<<2)		/* register = MNREGBIT \ REGBIT */
#define SIZEBIT  (1<<3)		/* sizing mnemonic = MNREGBIT \ SIZEBIT */
#define SEGM     15		/* 1st 4 bits reused for segment if !MNREGBIT */
#define RELBIT   (1<<4)		/* relative (= OBJ_A_MASK) */
#define FORBIT   (1<<5)		/* forward referenced */
#define IMPBIT   (1<<6)		/* imported (= OBJ_I_MASK) */
#define UNDBIT   (1<<7)		/* undefined */

/* pseudo-op routine numbers */
/* conditionals are first, this is used to test if op is a conditional */

#define ELSEOP      0
#define ELSEIFOP    1
#define ELSEIFCOP   2
#define ENDIFOP     3
#define IFOP        4
#define IFCOP       5
#define MAXCOND     6		/* limit of conditionals */

#define BLOCKOP     6
#define COMMOP      7
#define ENDOP       8
#define ENDBOP      9
#define ENTEROP    10
#define ENTRYOP    11
#define EQUOP      12
#define EXPORTOP   13
#define FAILOP     14
#define FCBOP      15
#define FCCOP      16
#define FDBOP      17
#define GETOP      18
#define IDENTOP    19
#define IMPORTOP   20
#define _LISTOP    21
#define LOCOP      22
#define _MACLISTOP 23
#define MACROOP    24
#define _MAPOP     25
#define ORGOP      26
#define RMBOP      27
#define SETOP      28
#define SETDPOP    29
#define _WARNOP    30

#ifdef I80386

/* further pseudo-ops */

#define BSSOP      31
#define COMMOP1    32
#define DATAOP     33
#define TEXTOP     34
#define USE16OP    35
#define USE32OP    36

/* machine-op routine numbers */

#define ARPL       37
#define BCC        38
#define BOUND      39
#define CALL       40
#define DIVMUL     41
#define ENTER      42
#define GROUP1     43
#define GROUP2     44
#define GROUP6     45
#define GROUP7     46
#define GROUP8     47
#define GvEv       48
#define IMUL       49
#define IN         50
#define INCDEC     51
#define INHER      52
#define INHER16    53
#define INHER32    54
#define INHER_A    55
#define INT        56
#define JCC        57
#define JCXZ       58
#define LEA        59
#define LOAD_FULL_POINTER 60
#define MOV        61
#define MOVX       62
#define NEGNOT     63
#define OUT        64
#define PUSHPOP    65
#define RET        66
#define RETF       67
#define SEG        68
#define SETCC      69
#define SH_DOUBLE  70
#define TEST       71
#define XCHG       72

/* further pseudo-ops */

#define BLKWOP     73
#define EVENOP     74
#define FQBOP      75
#define ALIGNOP    76

/* further machine-ops */

#define CALLI      77

/* yet further pseudo-ops */

#define LCOMMOP    78
#define LCOMMOP1   79

#endif				/* I80386 */

#ifdef MC6809

/* machine-op routine numbers */

#define ALL        31		/* all address modes allowed, like LDA */
#define ALTER      32		/* all but immediate, like STA */
#define IMMED      33		/* immediate only (ANDCC, ORCC) */
#define INDEXD     34		/* indexed (LEA's) */
#define INHER      35		/* inherent, like CLC or CLRA */
#define LONG       36		/* long branches */
#define SHORT      37		/* short branches */
#define SSTAK      38		/* S-stack (PSHS, PULS) */
#define SWAP       39		/* TFR, EXG */
#define USTAK      40		/* U-stack (PSHU,PULU) */

/* yet further pseudo-ops */

#define LCOMMOP    41

#endif

/* object code format (Introl) */

#define OBJ_SEGSZ_TWO  0x02	/* size 2 code for segment size descriptor */

#define OBJ_MAX_ABS_LEN  64	/* max length of chunk of absolute code */

#define OBJ_ABS        0x40	/* absolute code command */
#define OBJ_OFFSET_REL 0x80	/* offset relocation command */
#define OBJ_SET_SEG    0x20	/* set segment command */
#define OBJ_SKIP_1     0x11	/* skip with 1 byte count */
#define OBJ_SKIP_2     0x12	/* skip with 2 byte count */
#define OBJ_SKIP_4     0x13	/* skip with 4 byte count */
#define OBJ_SYMBOL_REL 0xC0	/* symbol relocation command */

#define OBJ_A_MASK     0x10	/* absolute bit(symbols) */
#if OBJ_A_MASK - RELBIT		/* must match internal format (~byte 1 -> 0) */
oops - RELBIT misplaced
#endif
#define OBJ_E_MASK     0x80	/* exported bit (symbols) */
#if OBJ_E_MASK - EXPBIT		/* must match internal format (byte 0 -> 0) */
oops - EXPBIT misplaced
#endif
#define OBJ_I_MASK     0x40	/* imported bit (symbols) */
#if OBJ_I_MASK - IMPBIT		/* must match internal format (byte 1 -> 0) */
oops - IMPBIT misplaced
#endif
#define OBJ_N_MASK     0x01	/* entry bit (symbols) */
#if OBJ_N_MASK - ENTBIT		/* must match internal format (byte 0 -> 1) */
oops - ENTBIT misplaced
#endif
#define OBJ_SA_MASK    0x20	/* size allocation bit (symbols) */
#define OBJ_SZ_ONE     0x40	/* size one code for symbol value */
#define OBJ_SZ_TWO     0x80	/* size two code for symbol value */
#define OBJ_SZ_FOUR    0xC0	/* size four code for symbol value */

#define OBJ_R_MASK     0x20	/* PC-rel bit (off & sym reloc commands) */
#define OBJ_SEGM_MASK  0x0F	/* segment mask (symbols, off reloc command) */

#define OBJ_OF_MASK    0x03	/* offset size code for symbol reloc */
#define OBJ_S_MASK     0x04	/* symbol number size code for symbol reloc */

#define SYMLIS_NAMELEN 26
#define SYMLIS_LEN (sizeof (struct sym_listing_s))

#define FILNAMLEN 64		/* max length of a file name */
#define LINLEN 256		/* max length of input line */
#define LINUM_LEN 5		/* length of formatted line number */

#define SPTSIZ 1024		/* number of symbol ptrs */
				/* pseudo-op flags */
#define POPHI 1			/* set to print hi byte of adr */
#define POPLO 2			/* to print lo byte of ADR */
#define POPLC 4			/* to print LC */
#define POPLONG 8		/* to print high word of ADR */
#define MAXBLOCK 8		/* max nesting level of BLOCK stack */
#define MAXGET 8		/* max nesting level of GET stack */
#define MAXIF 8			/* max nesting level of IF stack */
#define MACPSIZ (128/sizeof (struct schain_s))
				/* size of macro param buffer */
#define MAXMAC 8		/* max nesting level of macro stack */
#define NLOC 16			/* number of location counters */
#ifdef I80386
#define NO_SIB 0340		/* illegal sib (3 with 4) to mean no sib */
#endif

/* special segments */

#define BSSLOC 3
#define DATALOC 3
#define DPLOC 2
#define STRLOC 1
#define TEXTLOC 0
