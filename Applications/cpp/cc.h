
#ifndef P
#if __STDC__
#define P(x) x
#else
#define P(x) ()
#endif
#endif

extern void cfatal P((char*));
extern void cerror P((char*));
extern void cwarn P((char*));
extern FILE * open_include P((char*, char*, int));

extern FILE * curfile;
extern char   curword[];
extern char * c_fname;
extern int    c_lineno;
extern int    alltok;
extern int    dialect;

#define DI_KNR	1
#define DI_ANSI	2

extern int gettok P((void));

struct token_trans { char * name; int token; };
struct token_trans * is_ctok P((const char *str, unsigned int len));
struct token_trans * is_ckey P((const char *str, unsigned int len));

#define WORDSIZE	128
#define TK_WSPACE	256
#define TK_WORD 	257
#define TK_NUM  	258
#define TK_FLT  	259
#define TK_QUOT 	260
#define TK_STR  	261
#define TK_FILE 	262
#define TK_LINE 	263
#define TK_COPY 	264

#define TKS_CTOK  0x200
#define TKS_CKEY  0x300

#define TK_NE_OP	(TKS_CTOK+ 0)
#define TK_MOD_ASSIGN	(TKS_CTOK+ 1)
#define TK_AND_OP	(TKS_CTOK+ 2)
#define TK_AND_ASSIGN	(TKS_CTOK+ 3)
#define TK_MUL_ASSIGN	(TKS_CTOK+ 4)
#define TK_INC_OP	(TKS_CTOK+ 5)
#define TK_ADD_ASSIGN	(TKS_CTOK+ 6)
#define TK_DEC_OP	(TKS_CTOK+ 7)
#define TK_SUB_ASSIGN	(TKS_CTOK+ 8)
#define TK_PTR_OP	(TKS_CTOK+ 9)
#define TK_ELLIPSIS	(TKS_CTOK+10)
#define TK_DIV_ASSIGN	(TKS_CTOK+11)
#define TK_LEFT_OP	(TKS_CTOK+12)
#define TK_LEFT_ASSIGN	(TKS_CTOK+13)
#define TK_LE_OP	(TKS_CTOK+14)
#define TK_EQ_OP	(TKS_CTOK+15)
#define TK_GE_OP	(TKS_CTOK+16)
#define TK_RIGHT_OP	(TKS_CTOK+17)
#define TK_RIGHT_ASSIGN	(TKS_CTOK+18)
#define TK_XOR_ASSIGN	(TKS_CTOK+19)
#define TK_OR_ASSIGN	(TKS_CTOK+20)
#define TK_OR_OP	(TKS_CTOK+21)

#define TK_AUTO		(TKS_CKEY+ 0)
#define TK_BREAK	(TKS_CKEY+ 1)
#define TK_CASE		(TKS_CKEY+ 2)
#define TK_CHAR		(TKS_CKEY+ 3)
#define TK_CONST	(TKS_CKEY+ 4)
#define TK_CONTINUE	(TKS_CKEY+ 5)
#define TK_DEFAULT	(TKS_CKEY+ 6)
#define TK_DO		(TKS_CKEY+ 7)
#define TK_DOUBLE	(TKS_CKEY+ 8)
#define TK_ELSE		(TKS_CKEY+ 9)
#define TK_ENUM		(TKS_CKEY+10)
#define TK_EXTERN	(TKS_CKEY+11)
#define TK_FLOAT	(TKS_CKEY+12)
#define TK_FOR		(TKS_CKEY+13)
#define TK_GOTO		(TKS_CKEY+14)
#define TK_IF		(TKS_CKEY+15)
#define TK_INT		(TKS_CKEY+16)
#define TK_LONG		(TKS_CKEY+17)
#define TK_REGISTER	(TKS_CKEY+18)
#define TK_RETURN	(TKS_CKEY+19)
#define TK_SHORT	(TKS_CKEY+20)
#define TK_SIGNED	(TKS_CKEY+21)
#define TK_SIZEOF	(TKS_CKEY+22)
#define TK_STATIC	(TKS_CKEY+23)
#define TK_STRUCT	(TKS_CKEY+24)
#define TK_SWITCH	(TKS_CKEY+25)
#define TK_TYPEDEF	(TKS_CKEY+26)
#define TK_UNION	(TKS_CKEY+27)
#define TK_UNSIGNED	(TKS_CKEY+28)
#define TK_VOID		(TKS_CKEY+29)
#define TK_VOLATILE	(TKS_CKEY+30)
#define TK_WHILE	(TKS_CKEY+31)

#define MAX_INCLUDE 64	/* Nested includes */
#define MAX_DEFINE  64	/* Nested defines */

extern char * set_entry P((int,char*,void*));
extern void * read_entry P((int,char*));

struct define_item
{
   struct define_arg * next;
   char * name;
   int arg_count;	/* -1 = none; >=0 = brackets with N args */
   int in_use;		/* Skip this one for looking up #defines */
   int varargs;		/* No warning if unexpected arguments. */
   char value[1];	/* [arg,]*value */
};
