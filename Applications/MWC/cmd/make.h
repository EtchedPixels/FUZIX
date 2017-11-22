/*
 * make.h
 * Definitions and declarations for make.
 * Created due to the offended sensitivities of all MWC, 1-2-85.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ar.h>
#include <time.h>
#include <err.h>

/* Exit codes. */
#define	ALLOK	0	/* all ok, if -q option then all uptodate */
#define	ERROR	1	/* something went wrong */
#define	NOTUTD	2	/* with -q option, something is not uptodate */

/* Types. */
#define	T_UNKNOWN	0
#define	T_NODETAIL	1
#define	T_DETAIL	2
#define	T_DONE		3

/* Other manifest constants. */
#define	TRUE	(0 == 0)
#define	FALSE	(0 != 0)
#define	EOS	0200
#define	NUL	'\0'
#define	NBACKUP	2048
#define	NMACRONAME	48
#define	NTOKBUF	100
#define	READ	0	/* open argument for reading */

/* Macros. */
#define	Streq(a,b)	(strcmp(a,b) == 0)

/* Structures. */
typedef	struct token {
	struct token *next;
	char *value;
} TOKEN;

typedef struct macro {
	struct macro *next;
	char *value;
	char *name;
	int protected;
} MACRO;

typedef struct sym {
	struct sym *next;
	char *action;
	char *name;
	char *filename;
	struct dep *deplist;
	int type;
	time_t moddate;
} SYM;

typedef struct dep {
	struct dep *next;
	char *action;
	struct sym *symbol;
} DEP;

/* System dependencies. */

#define ACTIONFILE	"makeactions"
#define MACROFILE	"makemacros"
#define TDELIM		" \t\n=:;"

extern void die(const char *, ...);
extern void doerr(const char *, ...);
extern char *mmaloc(size_t);
extern void Usage(void);
extern int readc(void);
extern void putback(int);
extern void unreads(char *);
extern char *mexists(char *);
extern void define(char *, char *, int);
extern int ismacro(int);
extern int nextc(void);
extern char *extend(char *, int, char *, int);
extern int delim(char , char *);
extern void starttoken(void);
extern void addtoken(int);
extern void endtoken(void);
extern TOKEN *listtoken(char *, TOKEN *);
extern TOKEN *freetoken(TOKEN *);
extern void input(const char *);
extern void inlib(const char *);
extern void inpath(char *, ...);
extern time_t getmdate(char *);
extern int fexists(char *);
extern char *fpath(char *);
extern SYM *sexists(char *name);
extern SYM *dexists(char *name, DEP *dp);
extern SYM *lookup(char *);
extern DEP *adddep(char *, char *, DEP *);
extern void install(TOKEN *, TOKEN *, char *, int);
extern void make(SYM *);
extern void expand(char *);
extern void docmd0(SYM *, char *, char *, char *);
extern void docmd(SYM *, char *, char *, char *, char *, char *);
extern void doit(char *cmd);
extern void implicit(SYM *, char *, int);
extern void defalt(SYM *, char *);


/* end of make.h */
