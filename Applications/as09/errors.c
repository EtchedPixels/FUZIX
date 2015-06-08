
#include "syshead.h"
#include "const.h"
#include "errors.h"

/* Error codes. */

/* Syntax errors. */
char COMEXP[] =          "comma expected";
char DELEXP[] =          "delimiter expected";
char FACEXP[] =          "factor expected";
char IREGEXP[] =         "index register expected";
char LABEXP[] =          "label expected";
char LPEXP[] =           "left parentheses expected";
char OPEXP[] =           "opcode expected";
char RBEXP[] =           "right bracket expected";
char REGEXP[] =          "register expected";
char RPEXP[] =           "right parentheses expected";
char SPEXP[] =           "space expected";

/* Expression errors. */
char ABSREQ[] =          "absolute expression required";
char NONIMPREQ[] =       "non-imported expression required";
char RELBAD[] =          "relocation impossible";

/* Label errors. */
char ILLAB[] =           "illegal label";
char MACUID[] =          "MACRO used as identifier";
char MISLAB[] =          "missing label";
char MNUID[] =           "opcode used as identifier";
char REGUID[] =          "register used as identifier";
char RELAB[] =           "redefined label";
char UNBLAB[] =          "unbound label";
char UNLAB[] =           "undefined label";
char VARLAB[] =          "variable used as label";

/* Addressing errors. */
char ABOUNDS[] =         "address out of bounds";
char DBOUNDS[] =         "data out of bounds";
char ILLMOD[] =          "illegal address mode";
char ILLREG[] =          "illegal register";

/* Control structure errors. */
char ELSEBAD[] =         "no matching IF";
char ENDBBAD[] =         "no matching BLOCK";
char EOFBLOCK[] =        "end of file in BLOCK";
char EOFIF[] =           "end of file in IF";
char EOFLC[] =           "location counter was undefined at end";
char EOFMAC[] =          "end of file in MACRO";
char FAILERR[] =         "user-generated error";

/* Overflow errors. */
char BLOCKOV[] =         "BLOCK stack overflow";
char BWRAP[] =           "binary file wrap-around";
char COUNTOV[] =         "counter overflow";
char COUNTUN[] =         "counter underflow";
char GETOV[] =           "GET stack overflow";
char IFOV[] =            "IF stack overflow";

char LINLONG[] =         "line too long";
char MACOV[] =           "MACRO stack overflow";
char OBJSYMOV[] =        "object symbol table overflow";
char OWRITE[] =          "program overwrite";
char PAROV[] =           "parameter table overflow";
char SYMOV[] =           "symbol table overflow";
char SYMOUTOV[] =        "output symbol table overflow";

/* I/O errors. */
char OBJOUT[] =          "error writing object file";

/* Miscellaneous errors. */
char AL_AX_EAX_EXP[] =   "al ax or eax expected";
char CTLINS[] =          "control character in string";
char FURTHER[] =         "futher errors suppressed";
char ILL_IMM_MODE[] =    "illegal immediate mode";
char ILL_IND_TO_IND[] =  "illegal indirect to indirect";
char ILL_IND[] =         "illegal indirection";
char ILL_IND_PTR[] =     "illegal indirection from previous 'ptr'";
char ILL_SCALE[] =       "illegal scale";
char ILL_SECTION[] =     "illegal section";
char ILL_SEG_REG[] =     "illegal segment register";
char ILL_SOURCE_EA[] =   "illegal source effective address";
char ILL_SIZE[] =        "illegal size";
char IMM_REQ[] =         "immediate expression expected";
char INDEX_REG_EXP[] =   "index register expected";
char IND_REQ[] =         "indirect expression required";
char MISMATCHED_SIZE[] =  "mismatched size";
char NOIMPORT[] =        "no imports with binary file output";
char REENTER[] =         "multiple ENTER pseudo-ops";
char REL_REQ[] =         "relative expression required";
char REPEATED_DISPL[] =  "repeated displacement";
char SEGREL[] =          "segment or relocatability redefined";
char SEG_REG_REQ[] =     "segment register required";
char SIZE_UNK[] =        "size unknown";
char UNKNOWN_ESCAPE_SEQUENCE[] =  "unknown escape sequence";

char FP_REG_REQ[] =      "FP register required";
char FP_REG_NOT_ALLOWED[] =  "FP register not allowed";
char ILL_FP_REG[] =      "illegal FP register";
char ILL_FP_REG_PAIR[] =  "illegal FP register pair";
char JUNK_AFTER_OPERANDS[] =  "junk after operands";

char ALREADY[] =         "already defined";
char UNSTABLE_LABEL[] =  "label moved in last pass add -O?";

char REPNE_STRING[] =    "CMPS or SCAS expected";
char REP_STRING[] =      "string instruction expected";

/* Warnings. */
char CPUCLASH[] =        "instruction illegal for current cpu";
char SHORTB[] =          "short branch would do";
