
#ifndef _H_ERRORS
#define _H_ERRORS
/* Error codes. */

/* Syntax errors. */
EXTERN char COMEXP[];           /* "comma expected" */
EXTERN char DELEXP[];           /* "delimiter expected" */
EXTERN char FACEXP[];           /* "factor expected" */
EXTERN char IREGEXP[];          /* "index register expected" */
EXTERN char LABEXP[];           /* "label expected" */
EXTERN char LPEXP[];            /* "left parentheses expected" */
EXTERN char OPEXP[];            /* "opcode expected" */
EXTERN char RBEXP[];            /* "right bracket expected" */
EXTERN char REGEXP[];           /* "register expected" */
EXTERN char RPEXP[];            /* "right parentheses expected" */
EXTERN char SPEXP[];            /* "space expected" */

/* Expression errors. */
EXTERN char ABSREQ[];           /* "absolute expression required" */
EXTERN char NONIMPREQ[];        /* "non-imported expression required" */
EXTERN char RELBAD[];           /* "relocation impossible" */

/* Label errors. */
EXTERN char ILLAB[];            /* "illegal label" */
EXTERN char MACUID[];           /* "MACRO used as identifier" */
EXTERN char MISLAB[];           /* "missing label" */
EXTERN char MNUID[];            /* "opcode used as identifier" */
EXTERN char REGUID[];           /* "register used as identifier" */
EXTERN char RELAB[];            /* "redefined label" */
EXTERN char UNBLAB[];           /* "unbound label" */
EXTERN char UNLAB[];            /* "undefined label" */
EXTERN char VARLAB[];           /* "variable used as label" */

/* Addressing errors. */
EXTERN char ABOUNDS[];          /* "address out of bounds" */
EXTERN char DBOUNDS[];          /* "data out of bounds" */
EXTERN char ILLMOD[];           /* "illegal address mode" */
EXTERN char ILLREG[];           /* "illegal register" */

/* Control structure errors. */
EXTERN char ELSEBAD[];          /* "no matching IF" */
#define ELSEIFBAD       ELSEBAD
EXTERN char ENDBBAD[];          /* "no matching BLOCK" */
#define ENDIFBAD        ELSEBAD
EXTERN char EOFBLOCK[];         /* "end of file in BLOCK" */
EXTERN char EOFIF[];            /* "end of file in IF" */
EXTERN char EOFLC[];            /* "location counter was undefined at end" */
EXTERN char EOFMAC[];           /* "end of file in MACRO" */
EXTERN char FAILERR[];          /* "user-generated error" */

/* Overflow errors. */
EXTERN char BLOCKOV[];          /* "BLOCK stack overflow" */
EXTERN char BWRAP[];            /* "binary file wrap-around" */
EXTERN char COUNTOV[];          /* "counter overflow" */
EXTERN char COUNTUN[];          /* "counter underflow" */
EXTERN char GETOV[];            /* "GET stack overflow" */
EXTERN char IFOV[];             /* "IF stack overflow" */

EXTERN char LINLONG[];          /* "line too long" */
EXTERN char MACOV[];            /* "MACRO stack overflow" */
EXTERN char OBJSYMOV[];         /* "object symbol table overflow" */
EXTERN char OWRITE[];           /* "program overwrite" */
EXTERN char PAROV[];            /* "parameter table overflow" */
EXTERN char SYMOV[];            /* "symbol table overflow" */
EXTERN char SYMOUTOV[];         /* "output symbol table overflow" */

/* I/O errors. */
EXTERN char OBJOUT[];           /* "error writing object file" */

/* Miscellaneous errors. */
EXTERN char AL_AX_EAX_EXP[];    /* "al ax or eax expected" */
EXTERN char CTLINS[];           /* "control character in string" */
EXTERN char FURTHER[];          /* "futher errors suppressed" */
EXTERN char ILL_IMM_MODE[];     /* "illegal immediate mode" */
EXTERN char ILL_IND_TO_IND[];   /* "illegal indirect to indirect" */
EXTERN char ILL_IND[];          /* "illegal indirection" */
EXTERN char ILL_IND_PTR[];      /* "illegal indirection from previous 'ptr'" */
EXTERN char ILL_SCALE[];        /* "illegal scale" */
EXTERN char ILL_SECTION[];      /* "illegal section" */
EXTERN char ILL_SEG_REG[];      /* "illegal segment register" */
EXTERN char ILL_SOURCE_EA[];    /* "illegal source effective address" */
EXTERN char ILL_SIZE[];         /* "illegal size" */
EXTERN char IMM_REQ[];          /* "immediate expression expected" */
EXTERN char INDEX_REG_EXP[];    /* "index register expected" */
EXTERN char IND_REQ[];          /* "indirect expression required" */
EXTERN char MISMATCHED_SIZE[];  /* "mismatched size" */
EXTERN char NOIMPORT[];         /* "no imports with binary file output" */
EXTERN char REENTER[];          /* "multiple ENTER pseudo-ops" */
EXTERN char REL_REQ[];          /* "relative expression required" */
EXTERN char REPEATED_DISPL[];   /* "repeated displacement" */
EXTERN char SEGREL[];           /* "segment or relocatability redefined" */
EXTERN char SEG_REG_REQ[];      /* "segment register required" */
EXTERN char SIZE_UNK[];         /* "size unknown" */
EXTERN char UNKNOWN_ESCAPE_SEQUENCE[]; /* "unknown escape sequence" */

EXTERN char FP_REG_REQ[];       /* "FP register required" */
EXTERN char FP_REG_NOT_ALLOWED[]; /* "FP register not allowed" */
EXTERN char ILL_FP_REG[];       /* "illegal FP register" */
EXTERN char ILL_FP_REG_PAIR[];  /* "illegal FP register pair" */
EXTERN char JUNK_AFTER_OPERANDS[]; /* "junk after operands" */

EXTERN char ALREADY[];          /* "already defined" */
EXTERN char UNSTABLE_LABEL[];   /* "label moved in last pass add -O?" */

EXTERN char REPNE_STRING[];     /* "CMPS or SCAS expected" */
EXTERN char REP_STRING[];       /* "string instruction expected" */

/* Warnings. */
EXTERN char CPUCLASH[];         /* "instruction illegal for current cpu" */
EXTERN char SHORTB[];           /* "short branch would do" */

#endif
