/* Tokens */
#define T_SYMBOL	0x8000	/* Upwards */

/* Special control symbols */
#define T_EOF		0x7F00
#define T_INVALID	0x7F01
#define T_POT		0x7F02

/* Special case symbols */
#define T_SHLEQ		0x0110
#define T_SHREQ		0x0111
#define T_POINTSTO	0x0112
#define T_ELLIPSIS	0x0113

/* Two repeats of a symbol */
#define T_DOUBLESYM	0x0120
#define T_PLUSPLUS	0x0120
#define T_MINUSMINUS	0x0121
#define T_EQEQ		0x0122
#define T_LTLT		0x0123
#define T_GTGT		0x0124
#define T_OROR		0x0125
#define T_ANDAND	0x0126

/* Symbol followed by equals */
#define T_SYMEQ		0x0200
#define T_PLUSEQ	0x0200
#define T_MINUSEQ	0x0201
#define T_SLASHEQ	0x0202
#define T_STAREQ	0x0203
#define T_HATEQ		0x0204
#define T_BANGEQ	0x0205
#define T_OREQ		0x0206
#define T_ANDEQ		0x0207
#define T_PERCENTEQ	0x0208
#define T_LTEQ		0x0209
#define T_GTEQ		0x020A

/* Symbols with a semantic meaning - we map them to themselves for
   convenience */
#define T_UNI		0x0200
#define T_LPAREN	'('
#define T_RPAREN	')'
#define T_LSQUARE	'['
#define T_RSQUARE	']'
#define T_LCURLY	'{'
#define T_RCURLY	'}'
#define T_AND		'&'
#define T_STAR		'*'
#define T_SLASH		'/'
#define T_PERCENT	'%'
#define T_PLUS		'+'
#define T_MINUS		'-'
#define T_QUESTION	'?'
#define T_COLON		':'
#define T_HAT		'^'
#define T_LT		'<'
#define T_GT		'>'
#define T_OR		'|'
#define T_TILDE		'~'
#define T_BANG		'!'
#define T_EQ		'='
#define T_SEMICOLON	';'
#define T_DOT		'.'
#define T_COMMA		','

/* We process strings and quoting of strings so " ' and \ are not seen */

/* The C language keywords */		
#define T_KEYWORD	0x1000

/* Type keywords */
#define T_CHAR		0x1000
#define T_DOUBLE	0x1001
#define T_ENUM		0x1002
#define T_FLOAT		0x1003
#define T_INT		0x1004
#define T_LONG		0x1005
#define T_SHORT		0x1006
#define T_SIGNED	0x1007
#define T_STRUCT	0x1008
#define T_UNION		0x1009
#define T_UNSIGNED	0x100A
#define T_VOID		0x100B
/* Storage classes */
#define T_AUTO		0x100C
#define T_EXTERN	0x100D
#define T_REGISTER	0x100E
#define	T_STATIC	0x100F
/* Modifiers */
#define T_CONST		0x1010
#define T_VOLATILE	0x1011
/* Other keywords */
#define T_BREAK		0x1012
#define T_CASE		0x1013
#define T_CONTINUE	0x1014
#define T_DEFAULT	0x1015
#define T_DO		0x1016
#define T_ELSE		0x1017
#define T_FOR		0x1018
#define T_GOTO		0x1019
#define T_IF		0x101A
#define T_RETURN	0x101B
#define T_SIZEOF	0x101C
#define T_SWITCH	0x101D
#define T_TYPEDEF	0x101E
#define T_WHILE		0x101F
#define T_RESTRICT	0x1020	/* We treat this as a nop */

/* Encodings for tokenized constants */
/* These are followed by a 4 byte little endian value */
#define T_INTVAL	0x1100
#define T_UINTVAL	0x1101
#define T_LONGVAL	0x1102
#define T_ULONGVAL	0x1103
#define T_FLOATVAL	0x1104
/* This is followed by a literal bytestream terminated by a 0. 0 and 255 in the
   stream are encoded as 255,0 and 255,255. */
#define T_STRING	0x1105
/* End marker for strings (mostly a dummy for convenience) */
#define T_STRING_END	0x1106

/* Encodings that are used internally for nodes but have no actual equivalent
   token */
#define T_CAST		0x1200		/* (int *) etc */
#define T_CONSTANT	0x1201		/* A numeric value */
#define T_NAME		0x1202		/* name/offset for global or static */
#define T_LOCAL		0x1203		/* name/offset for auto */
#define T_LABEL		0x1204		/* address of a literal */
#define T_ARGUMENT	0x1205		/* name/offset for argument */
#define T_DEREF		0x1206		/* *foo (as T_STAR is multiply) */
#define T_ADDROF	0x1207		/* &lval (as T_AND is boolean and) */
#define T_NULL		0x1208		/* An empty expression */
#define T_NEGATE	0x1209		/* Unary negation (T_MINUS is binary) */
#define T_FUNCCALL	0x120A		/* Function call */
#define T_BOOL		0x120B		/* Convert working reg to 0/1 */
#define T_PAD		0x120C		/* Reserve memory in data/bss */
#define T_CLEANUP	0x120D		/* Function call post stack clean */
#define T_CASELABEL	0x120E		/* A case label */
#define T_ARGCOMMA	0x120F		/* Links arguments to a call */
#define T_REG		0x1210		/* name/offset for a register */

#define T_USER		0x2000		/* Tokens for code generators */

/* Used to pass line number information */
#define T_LINE		0x3FFF
