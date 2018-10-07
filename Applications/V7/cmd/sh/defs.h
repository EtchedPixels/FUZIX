/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/times.h>
#include <errno.h>

/* Figure out if we have to deal with bigger buffers specially due to cc65
   limits. Really cc65 needs fixing. */
#if defined(__CC65__) && defined(BUILD_FSH)
#define static6502	static
#else
#define static6502
#endif

/*
 *	UNIX shell
 */

/* error exits from various parts of shell */
#define ERROR	1
#define SYNBAD	2
#define SIGFAIL 3
#define SIGFLG	0200

/* command tree */
#define FPRS	020
#define FINT	040
#define FAMP	0100
#define FPIN	0400
#define FPOU	01000
#define FPCL	02000
#define FCMD	04000
#define COMMSK	017

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11

/* execute table */
#define SYSSET	1
#define SYSCD	2
#define SYSEXEC	3
#define SYSLOGIN 4
#define SYSTRAP	5
#define SYSEXIT	6
#define SYSSHFT 7
#define SYSWAIT	8
#define SYSCONT 9
#define SYSBREAK 10
#define SYSEVAL 11
#define SYSDOT	12
#define SYSRDONLY 13
#define SYSTIMES 14
#define SYSXPORT 15
#define SYSNULL 16
#define SYSREAD 17
#define SYSTST	18
#define	SYSUMASK	19

/* used for input and output of shell */
#if 0 /* V7 */
#define INIO 10
#define OTIO 11
#else
/* FUZIX default is currently 10 fds per proc.. probably should move to 16 */
#define INIO 8
#define OTIO 9
#endif

/*io nodes*/
#define USERIO	10
#define IOUFD	15
#define IODOC	16
#define IOPUT	32
#define IOAPP	64
#define IOMOV	128
#define IORDW	256
#define INPIPE	0
#define OTPIPE	1

/* arg list terminator */
#define ENDARGS	0

#include	"mac.h"
#include	"mode.h"
#include	"name.h"


/* result type declarations */
extern void *setbrk(intptr_t);
extern char **sh_setenv(void);


#define attrib(n,f)	(n->namflg |= f)
#define round(a,b)	(((int)((ADR(a)+b)-1))&~((b)-1))
#define closepipe(x)	(close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a,b)		(cf(a,b)==0)
#define max(a,b)	((a)>(b)?(a):(b))
#define assert(x)	;

/* temp files and io */
extern UFD output;
extern int ioset;
extern IOPTR iotemp;		/* files to be deleted sometime */
extern IOPTR iopend;		/* documents waiting to be read at NL */

/* substitution */
extern int dolc;
extern const char **dolv;
extern DOLPTR argfor;
extern ARGPTR gchain;

/* stack */
#define		BLK(x)	((BLKPTR)(x))
#define		BYT(x)	((BYTPTR)(x))
#define		STK(x)	((STKPTR)(x))
#define		ADR(x)	((char *)(x))

/* stak stuff */
#include	"stak.h"

/* string constants */
extern const char atline[];
extern const char readmsg[];
extern const char colon[];
extern const char minus[];
extern const char nullstr[];
extern const char sptbnl[];
extern const char unexpected[];
extern const char endoffile[];
extern const char synmsg[];

/* name tree and words */
extern SYSTAB reserved;
extern int wdval;
extern int wdnum;
extern ARGPTR wdarg;
extern int wdset;
extern BOOL reserv;

/* prompting */
extern const char stdprompt[];
extern const char supprompt[];
extern const char profile[];

/* built in names */
extern NAMNOD fngnod;
extern NAMNOD ifsnod;
extern NAMNOD homenod;
extern NAMNOD mailnod;
extern NAMNOD pathnod;
extern NAMNOD ps1nod;
extern NAMNOD ps2nod;

/* special names */
extern char flagadr[10];
extern char * cmdadr;
extern char * exitadr;
extern char * dolladr;
extern char * pcsadr;
extern char * pidadr;

extern const char defpath[];

/* names always present */
extern const char mailname[];
extern const char homename[];
extern const char pathname[];
extern const char fngname[];
extern const char ifsname[];
extern const char ps1name[];
extern const char ps2name[];

/* transput */
extern CHAR tmpout[];
extern char *tempfile;
extern int serial;
#define		TMPNAM 7
extern FILE standin;
#define input	(standin->fdes)
#define eof	(standin->feof)
extern int peekc;
extern const char *comdiv;
extern const char devnull[];

/* flags */
#define		noexec	01
#define		intflg	02
#define		prompt	04
#define		setflg	010
#define		errflg	020
#define		ttyflg	040
#define		forked	0100
#define		oneflg	0200
#define		rshflg	0400
#define		waiting	01000
#define		stdflg	02000
#define		execpr	04000
#define		readpr	010000
#define		keyflg	020000
extern int flags;

/* error exits from various parts of shell */
#include	<setjmp.h>
extern jmp_buf subshell;
extern jmp_buf errshell;

/* fault handling */
#include	"brkincr.h"
extern POS brkincr;

#define MINTRAP	0
#define MAXTRAP	17

#define INTR	2
#define QUIT	3
#define MEMF	11
#define ALARM	14
#define KILL	15
#define TRAPSET	2
#define SIGSET	4
#define SIGMOD	8

extern BOOL trapnote;
extern char * trapcom[];
extern BOOL trapflg[];

/* name tree and words */
extern char * *environ;
extern CHAR numbuf[];
extern const char export[];
extern const char readonly[];

/* execflgs */
extern int exitval;
extern BOOL execbrk;
extern int loopcnt;
extern int breakcnt;

/* messages */
extern const char mailmsg[];
extern const char coredump[];
extern const char badopt[];
extern const char badparam[];
extern const char badsub[];
extern const char nospace[];
extern const char notfound[];
extern const char badtrap[];
extern const char baddir[];
extern const char badshift[];
extern const char illegal[];
extern const char restricted[];
extern const char execpmsg[];
extern const char notid[];
extern const char wtfailed[];
extern const char badcreate[];
extern const char piperr[];
extern const char badopen[];
extern const char badnum[];
extern const char arglist[];
extern const char txtbsy[];
extern const char toobig[];
extern const char badexec[];
extern const char notfound[];
extern const char badfile[];

#include	"ctype.h"

/* args.c */
extern int options(int argc, const char *argv[]);
extern void setargs(const char *argi[]);
extern DOLPTR freeargs(DOLPTR blk);
extern void clearup(void);
extern DOLPTR useargs(void);
/* blok.c */
void blokinit(void);
ADDRESS alloc(POS nbytes);
extern void addblok(POS reqd);
extern void sh_free(void *ap);
extern int chkbptr(BLKPTR ptr);
/* builtin.c */
extern int builtin(int argn, char * *cmd);
/* cmd.c */
extern TREPTR makefork(int flgs, TREPTR i);
extern TREPTR cmd(int sym, int flg);
/* error.c */
extern void exitset(void);
extern void sigchk(void);
extern void failed(const char *s1, const char *s2);
extern void error(const char *s);
extern void exitsh(int xno);
extern void done(void);
extern void rmtemp(IOPTR base);
/* expand.c */
extern int expand(char *as, int rflg);
extern int gmatch(register char *s, register char *p);
extern void makearg(register char *args);
/* fault.c */
extern void fault(register int sig);
extern void stdsigs(void);
extern sighandler_t ignsig(int n);
extern void getsig(int n);
extern void oldsigs(void);
extern void clrsig(int i);
extern void chktrap(void);
/* io.c */
extern void initf(UFD fd);
extern int estabf(register const char *s);
extern void push(FILE af);
extern int pop(void);
extern void chkpipe(int *pv);
extern int chkopen(const char *idf);
extern void sh_rename(register int f1, register int f2);
extern int create(const char *s);
extern int tmpfil(void);
extern void copy(IOPTR ioparg);
/* macro.c */
extern char *macro(char *as);
extern void subst(int in, int ot);
/* main.c */
extern int main(int c, const char *v[]);
extern void chkpr(char eor);
extern void settmp(void);
extern void Ldup(register int fa, register int fb);
extern void gratuitous_call(void);
/* name.c */
extern int syslook(char *w, SYSTAB syswds);
extern void setlist(register ARGPTR arg, int xp);
extern void setname(char *argi, int xp);
extern void replace(char **a, const char *v);
extern void dfault(NAMPTR n, const char *v);
extern void assign(NAMPTR n, const char *v);
extern int readvar(char **names);
extern void assnum(char **p, int i);
extern char *make(const char *v);
extern NAMPTR lookup(register char *nam);
extern void namscan(void (*fn)(NAMPTR));
extern void printnam(NAMPTR n);
extern void exname(register NAMPTR n);
extern void printflg(register NAMPTR n);
extern void sh_getenv(void);
extern void countnam(NAMPTR n);
extern void pushnam(NAMPTR n);
extern char **sh_setenv(void);
/* print.c */
extern void newline(void);
extern void blank(void);
extern void prp(void);
extern void prs(const char *as);
extern void prc(char c);
extern void prt(clock_t t);
extern void prn(int n);
extern void itos(int n);
extern int stoi(const char *icp);
/* service.c */
extern void initio(IOPTR iop);
extern const char *getpath(const char *s);
extern int pathopen(const char *path, const char *name);
extern const char *catpath(register const char *path, const char *name);
extern void execa(const char **at);
extern void postclr(void);
extern void post(int pcsid);
extern void await(int i);
extern void trim(char *at);
extern char *mactrim(char *s);
extern char **scan(int argn);
extern int getarg(COMPTR ac);
/* stak.c */
extern char * getstak(int asize);
extern char * locstak(void);
extern char * savstak(void);
extern char * endstak(register char * argp);
extern void tdystak(register char * x);
extern void stakchk(void);
extern char *cpystak(const char *x);
/* string.c */
extern char *movstr(register const char *a, register char *b);
extern int any(char c, const char *s);
extern int cf(register const char *s1, register const char *s2);
extern int length(const char *as);
/* word.c */
extern int word(void);
extern int nextc(char quote);
extern int readc(void);
/* xec.c */
extern int execute(TREPTR argt, int execflg, int *pf1, int *pf2);
extern void execexp(char *s, UFD f);
