int advrestore(char *hdr, int hlen, char *save, int slen);
int advsave(char *hdr, int hlen, char *save, int slen);
int checkverb(int *verbs);
void complement(char *adr, int len);
void db_init(char *name);
int db_restart(void);
int db_restore(void);
int db_save(void);
int decode(int ch);
void display_picture(void);
void error(char *msg);
int execute(int code);
void exe_one(void);
int fill_action_array(void);
int findaction(int *verbs, int preposition, int flag);
int findprop(int obj, int prop);
int findword(char *word);
int getabyte(int act, int off);
int getafield(int act, int off);
int getaloc(int n);
int getboperand(void);
int getbyte(int n);
int getcbyte(int n);
int getchr(void);
int getch(void);
int getcword(int n);
int getdword(char *p);
void get_block(unsigned int blk);
int get_line(void);
int getnoun(void);
int getofield(int obj, int off);
int getoloc(int n);
int getp(int obj, int prop);
int getrand(int n);
int getvalue(int n);
int getverb(void);
int getwloc(int n);
int getwoperand(void);
int getword(int n);
int get_word(void);
int hasadjective(int obj, int adjective);
int hasnoun(int obj, int noun);
int haspreposition(int act, int preposition);
int hasverb(int act, int *verbs);
int inlist(int link, int word);
int match(int obj, int noun, int *adjs);
int msg_byte(void);
void msg_init(int fd, int base);
void msg_open(unsigned int msg);
int next(void);
void parse_error(void);
int parse1(void);
int parse(void);
void play(void);
void pnumber(int n);
void print(int msg);
void putchr(char ch);
int putdword(char *p, int w);
int putofield(int obj, int off, int val);
int putword(int n, int w);
int rand(void);
void range(char *what, int n);
int setp(int obj, int prop, int val);
void setrand(long n);
int setvalue(int n, int v);
void show_noun(int n);
int single(void);
int skip_spaces(void);
int spacep(int ch);
void trm_chr(int ch);
void trm_done(void);
void trm_eol(void);
char *trm_get(char *line);
void trm_init(int rows, int cols, char *name);
void trm_shutdown(char *textport);
void trm_str(const char *str);
void trm_wait(void);
void trm_word(void);
void trm_xstr(char *str);
char *trm_line(char *line);
int vowel(int msg);
void waitch(void);
int wtype(int wrd);

/* useful definitions */
#define TRUE		(int)1
#define FALSE		(int)0
#define EOS		'\0'

/* program limits */
#define STKSIZE		(int)500

/* code completion codes */
#define FINISH		(int)1
#define CHAIN		(int)2
#define ABORT		(int)3

/* useful constants */
#define T	-1L
#define NIL	0L
#define WRDSIZE	(int)6

/* data structure version number */
#define VERSION		((int)102)

/* file header offsets */
#define HDR_LENGTH	((int)0)	/* length of header in bytes */
#define HDR_MAGIC	((int)2)	/* magic information (6 bytes) */
#define HDR_VERSION	((int)8)	/* data structure version number */
#define HDR_ANAME	((int)10)	/* adventure name (18 bytes) */
#define HDR_AVERSION	((int)28)	/* adventure version number */
#define HDR_WTABLE	((int)30)	/* offset to word table */
#define HDR_WTYPES	((int)32)	/* offset to word type table */
#define HDR_OTABLE	((int)34)	/* offset to object table */
#define HDR_ATABLE	((int)36)	/* offset to action table */
#define HDR_VTABLE	((int)38)	/* offset to variable table */
#define HDR_DBASE	((int)40)	/* offset to base of data space */
#define HDR_CBASE	((int)42)	/* offset to base of code space */
#define HDR_DATBLK	((int)44)	/* first data block */
#define HDR_MSGBLK	((int)46)	/* first message text block */
#define HDR_INIT	((int)48)	/* initialization code */
#define HDR_UPDATE	((int)50)	/* update code */
#define HDR_BEFORE	((int)52)	/* code to execute before verb handler */
#define HDR_AFTER	((int)54)	/* code to execute after verb handler */
#define HDR_ERROR	((int)56)	/* error handler code */
#define HDR_SAVE	((int)58)	/* save area offset */
#define HDR_SLEN	((int)60)	/* save area length */
#define HDR_SIZE	((int)62)	/* size of header */

/* word types */
#define WT_UNKNOWN	((int)0)
#define WT_VERB		((int)1)
#define WT_NOUN		((int)2)
#define WT_ADJECTIVE	((int)3)
#define WT_PREPOSITION	((int)4)
#define WT_CONJUNCTION	((int)5)
#define WT_ARTICLE	((int)6)

/* object fields */
#define O_CLASS		((int)0)
#define O_NOUNS		((int)2)
#define O_ADJECTIVES	((int)4)
#define O_NPROPERTIES	((int)6)
#define O_PROPERTIES	((int)8)
#define O_SIZE		((int)8)

/* action fields */
#define A_VERBS		((int)0)
#define A_PREPOSITIONS	((int)2)
#define A_FLAG		((int)4)
#define A_MASK		((int)5)
#define A_CODE		((int)6)
#define A_SIZE		((int)8)

/* link fields */
#define L_DATA		((int)0)
#define L_NEXT		((int)2)
#define L_SIZE		((int)4)

/* property flags */
#define P_CLASS		(int)0x8000	/* class property */

/* action flags */
#define A_ACTOR		(int)0x01	/* actor */
#define A_DOBJECT	(int)0x02	/* direct object */
#define A_IOBJECT       (int)0x04	/* indirect object */

/* opcodes */
#define OP_BRT		(int)0x01	/* branch on true */
#define OP_BRF		(int)0x02	/* branch on false */
#define OP_BR		(int)0x03	/* branch unconditionally */
#define OP_T		(int)0x04	/* load top of stack with t */
#define OP_NIL		(int)0x05	/* load top of stack with nil */
#define OP_PUSH		(int)0x06	/* push nil onto stack */
#define OP_NOT		(int)0x07	/* logical negate top of stack */
#define OP_ADD		(int)0x08	/* add two numeric expressions */
#define OP_SUB		(int)0x09	/* subtract two numeric expressions */
#define OP_MUL		(int)0x0A	/* multiply two numeric expressions */
#define OP_DIV		(int)0x0B	/* divide two numeric expressions */
#define OP_REM		(int)0x0C	/* remainder of two numeric expressions */
#define OP_BAND		(int)0x0D	/* bitwise and of two numeric expressions */
#define OP_BOR		(int)0x0E	/* bitwise or of two numeric expressions */
#define OP_BNOT		(int)0x0F	/* bitwise not of two numeric expressions */
#define OP_LT		(int)0x10	/* less than */
#define OP_EQ		(int)0x11	/* equal to */
#define OP_GT		(int)0x12	/* greater than */
#define OP_LIT		(int)0x13	/* load literal */
#define OP_VAR		(int)0x14	/* load a variable value */
#define OP_GETP		(int)0x15	/* get the value of an object property */
#define OP_SETP		(int)0x16	/* set the value of an object property */
#define OP_SET		(int)0x17	/* set the value of a variable */
#define OP_PRINT	(int)0x18	/* print messages */
#define OP_TERPRI	(int)0x19	/* terminate the print line */
#define OP_PNUMBER	(int)0x1A	/* print a number */
#define OP_FINISH	(int)0x1B	/* finish handling this command */
#define OP_CHAIN	(int)0x1C	/* chain to the next handler */
#define OP_ABORT	(int)0x1D	/* abort this command */
#define OP_EXIT		(int)0x1E	/* exit the program */
#define OP_RETURN	(int)0x1F	/* return from interpreter */
#define OP_CALL		(int)0x20	/* call a function */
#define OP_SVAR		(int)0x21	/* int load a variable */
#define OP_SSET		(int)0x22	/* short set a variable */
#define OP_SPLIT	(int)0x23	/* short load a positive literal */
#define OP_SNLIT	(int)0x24	/* short load a negative literal */
#define OP_YORN		(int)0x25	/* yes-or-no predicate */
#define OP_SAVE		(int)0x26	/* save data structures */
#define OP_RESTORE	(int)0x27	/* restore data structures */
#define OP_ARG		(int)0x28	/* load an argument value */
#define OP_ASET		(int)0x29	/* set an argument value */
#define OP_TMP		(int)0x2A	/* load a temporary variable value */
#define OP_TSET		(int)0x2B	/* set a temporary variable */
#define OP_TSPACE	(int)0x2C	/* allocate temporary variable space */
#define OP_CLASS	(int)0x2D	/* get the class of an object */
#define OP_MATCH	(int)0x2E	/* match a noun phrase with an object */
#define OP_PNOUN	(int)0x2F	/* print a noun phrase */
#define OP_RESTART	(int)0x30	/* restart the current game */
#define OP_RAND		(int)0x31	/* generate a random number */
#define OP_RNDMIZE	(int)0x32	/* seed the random number generator */
#define OP_SEND		(int)0x33	/* send a message to an object */
#define OP_VOWEL	(int)0x34	/* check for vowel beginning string */

#define OP_XVAR		(int)0x40	/* extra int load a variable */
#define OP_XSET		(int)0x60	/* extra short set a variable */
#define OP_XPLIT	(int)0x80	/* extra short load a positive literal */
#define OP_XNLIT	(int)0xC0	/* extra short load a negative literal */

/* builtin variables */
#define V_ACTOR		(int)1	/* actor noun phrase number */
#define V_ACTION	(int)2	/* action from parse */
#define V_DOBJECT	(int)3	/* first direct object noun phrase number */
#define V_NDOBJECTS	(int)4	/* number of direct object noun phrases */
#define V_IOBJECT	(int)5	/* indirect object noun phrase number */
#define V_OCOUNT	(int)6	/* total object count */
