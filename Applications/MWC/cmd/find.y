%{
/*
 * Find all files in the given
 * directory hierarchies that
 * satisfy the given expression
 * primaries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <grp.h>
#include <pwd.h>
#include "findnode.h"

#define DIRSIZ	30		/* FIXME */

#define	NPRIM	(sizeof(primaries)/sizeof(primaries[0]))
#define	NARG	50
#define	NRECUR	14		/* Maximum recursion depth before forking */
#define	NFNAME	600		/* size of filename buffer */
#define	FILEARG	((char *)EOF)
#define	DAYSEC	(60L*60L*24L)	/* seconds in a day */
#define	inode(f,v)	lnode(FUN,f,v,NULL)
#define	snode(f,s)	lnode(FUN,f,0,s)

#define YYSTYPE	NODE *

NODE	*code;
int	seflag;			/* Set if a side effect (print, exec) found */

char	*next(void);
NODE	*bnode(int, NODE *, NODE *);
NODE	*enode(int type);
NODE	*lnode(int op, int (*fn)(NODE *), int val, char *str);
NODE	*nnode(int (*fun)(NODE *));
NODE	*onode(int (*fun)(NODE *));
NODE	*getuser(void);
NODE	*getgroup(void);
NODE	*getnewer(void);
int	xname(NODE *np);
int	xperm(NODE *np);
int	xtype(NODE *np);
int	xlinks(NODE *np);
int	xuser(NODE *np);
int	xgroup(NODE *np);
int	xsize(NODE *np);
int	xinum(NODE *np);
int	xatime(NODE *np);
int	xctime(NODE *np);
int	xmtime(NODE *np);
int	xnewer(NODE *np);
int	xexec(NODE *np);
int	xprint(NODE *np);
int	xnop(NODE *np);
%}
%start	command


%left	OR
%left	AND
%left	'!'
%token	NAME PERM TYPE LINKS USER GROUP SIZE INUM
%token	ATIME CTIME MTIME EXEC OK PRINT NEWER FUN NOP

%%

command:
	exp '\n'		{ if (seflag)
					code = $1; else
					code = bnode(AND,$1,snode(xprint,NULL));
				  return 0;
				}
      | '\n'			{ code = snode(xprint, NULL); return 0; }
	;

exp:
        '(' exp ')'		{ $$ = $2; }
      | '!' exp			{ $$ = bnode('!', $2, NULL); }
      | exp OR exp		{ $$ = bnode(OR, $1, $3); }
      | exp AND exp		{ $$ = bnode(AND, $1, $3); }
      | NAME  			{ $$ = snode(xname, next()); }
      | PERM			{ $$ = onode(xperm); }
      | TYPE			{ $$ = snode(xtype, next()); }
      | LINKS			{ $$ = nnode(xlinks); }
      | USER			{ $$ = getuser(); }
      | GROUP			{ $$ = getgroup(); }
      | SIZE			{ $$ = nnode(xsize); }
      | INUM			{ $$ = nnode(xinum); }
      | ATIME			{ $$ = nnode(xatime); }
      | CTIME			{ $$ = nnode(xctime); }
      | MTIME			{ $$ = nnode(xmtime); }
      | EXEC			{ $$ = enode(0); }
      | OK			{ $$ = enode(1); }
      | PRINT			{ $$ = snode(xprint, NULL); seflag++; }
      | NEWER			{ $$ = getnewer(); }
      | NOP			{ $$ = snode(xnop, NULL); seflag++; }
	;

%%
struct	primary	{
	char	*p_name;
	int	p_lval;
}	primaries[] = {
	{ "-name", NAME },
	{ "-perm", PERM },
	{ "-type", TYPE },
	{ "-links", LINKS },
	{ "-user", USER },
	{ "-group", GROUP },
	{ "-size", SIZE },
	{ "-inum", INUM },
	{ "-atime", ATIME },
	{ "-ctime", CTIME },
	{ "-mtime", MTIME },
	{ "-exec", EXEC },
	{ "-ok", OK },
	{ "-print", PRINT },
	{ "-newer", NEWER }, 
	{ "-nop", NOP },
	{ "-o", OR },
	{ "-a", AND },
};

char	**gav;
int	gac;
int	depth;			/* Recursive depth */

struct	stat	sb;
char	fname[NFNAME];
const char *prompt;

char	toodeep[] = "directory structure too deep to traverse";
char	nospace[] = "out of memory";

time_t	curtime;

char	*buildname(struct dirent *dp, char *ep);
int	execute(NODE *np);
void	find(char *dir);
void	fentry(char *ep, struct stat *sbp);
void	ffork(char *ep, struct stat *sbp);
void	usage(void);

int main(int argc, char *argv[])
{
	register int i;
	register char *ap;
	register int eargc;

	for (i=1; i<argc; i++) {
		ap = argv[i];
		if (*ap == '-')
			break;
		if (ap[1]=='\0' && (*ap=='!' || *ap=='('))
			break;
	}
	if ((eargc=i) < 2)
		usage();
	gav = argv+i;
	gac = argc-i;
	yyparse();
	time(&curtime);
	if ((prompt = getenv("PS1")) == NULL)
		prompt = "> ";
	for (i=1; i<eargc; i++)
		find(argv[i]);
}

/*
 * Lexical analyser
 */
int yylex(void)
{
	static int binop = 0;
	static int ntoken = 0;
	register char *ap;
	struct primary *pp;
	register int token;

	if (ntoken) {
		token = ntoken;
		ntoken = 0;
	} else if ((ap = next()) == NULL)
		token = '\n';
	else if (ap[1] == '\0')
		token = ap[0];
	else if (*ap == '-') {
		for (pp = primaries; pp <= &primaries[NPRIM-1]; pp++)
			if (strcmp(pp->p_name, ap) == 0) {
				token = pp->p_lval;
				break;
			}
		if (pp > &primaries[NPRIM-1])
			errx(1, "`%s' is an illegal primary", ap);
	} else
		errx(1, "Illegal expression %s\n", ap);
	if (binop && token!=')' && token!='\n' && token!=OR && token!=AND) {
		binop = 0;
		ntoken = token;
		return (AND);
	}
	if (token!=OR && token!=AND && token!='!' && token!='\n' && token!='(')
		binop = 1; else
		binop = 0;
	return (token);
}

void yyerror(const char *p)
{
	fprintf(stderr, "Primary expression syntax error\n");
	usage();
}

/*
 * Return the next argument from the arg list.
 */
char *next(void)
{
	if (gac < 1)
		return (NULL);
	gac--;
	return (*gav++);
}

/*
 * Produce a node consisting
 * of an octal number.
 */
NODE *onode(int (*fun)(NODE *))
{
	register char *ap;
	register int num;
	register NODE *np;
	register int type;
	char *aap;

	if ((ap = next()) == NULL)
		errx(1, "Missing octal permission");
	aap = ap;
	if (*ap == '-') {
		ap++;
		type = -1;
	} else
		type = 0;
	num = 0;
	while (*ap>='0' && *ap<='7')
		num = num*8 + *ap++-'0';
	if (*ap != '\0')
		errx(1, "%s: bad octal permission", aap);
	np = inode(fun, num);
	np->n_un.n_val = num;
	np->n_type = type;
	return (np);
}

/*
 * Get a number -- it also may be
 * prefixed by `+' or `-' to
 * represent quantities greater or
 * less.
 */
NODE *nnode(int (*fun)(NODE *))
{
	register char *ap;
	register int num = 0;
	register int type = 0;
	register NODE *np;
	char *aap;

	if ((ap = next()) == NULL)
		errx(1, "Missing number");
	aap = ap;
	if (*ap == '+') {
		type = 1;
		ap++;
	} else if (*ap == '-') {
		type = -1;
		ap++;
	}
	while (*ap>='0' && *ap<='9')
		num = num*10 + *ap++ - '0';
	if (*ap != '\0')
		errx(1, "%s: invalid number", aap);
	np = inode(fun, num);
	np->n_type = type;
	return (np);
}

/*
 * Get a user name or number.
 */
NODE *getuser(void)
{
	register struct passwd *pwp;
	register char *cp;
	register int uid;

	if ((cp = next()) == NULL)
		errx(1, "Missing username");
	if (*cp>='0' && *cp<='9')
		uid = atoi(cp);
	else {
		if ((pwp = getpwnam(cp)) == NULL)
			errx(1, "%s: bad user name", cp);
		uid = pwp->pw_uid;
	}
	return (lnode(FUN, xuser, uid, NULL));
}

/*
 * Get group
 */
NODE *getgroup(void)
{
	register struct group *grp;
	register char *cp;
	register int gid;

	if ((cp = next()) == NULL)
		errx(1, "Missing group name");
	if (*cp>='0' && *cp<='9')
		gid = atoi(cp);
	else {
		if ((grp = getgrnam(cp)) == NULL)
			errx(1, "%s: bad group name", cp);
		gid = grp->gr_gid;
	}
	return (lnode(FUN, xgroup, gid, NULL));
}

/*
 * Get the time for the file used in
 * the `-newer' primary.
 */
NODE *getnewer(void)
{
	register NODE *np;
	register char *fn;

	if ((fn = next()) == NULL)
		errx(1, "Missing filename for `-newer'");
	if (stat(fn, &sb) < 0)
		errx(1, "%s: nonexistent", fn);
	np = inode(xnewer, 0);
	np->n_un.n_time = sb.st_mtime;
	return (np);
}

/*
 * Build an expression tree node (non-leaf).
 */
NODE *bnode(int op, NODE *left, NODE *right)
{
	register NODE *np;

	if ((np = malloc(sizeof (NODE))) == NULL)
		errx(1, nospace);
	np->n_op = op;
	np->n_left = left;
	np->n_right = right;
	np->n_un.n_val = 0;
	return (np);
}

/*
 * Build a leaf node in expression tree.
 */
NODE *lnode(int op, int (*fn)(NODE *), int val, char *str)
{
	register NODE *np;

	if ((np = malloc(sizeof (NODE))) == NULL)
		errx(1, nospace);
	np->n_left = np->n_right = NULL;
	np->n_op = op;
	np->n_fun = fn;
	if (str != NULL)
		np->n_un.n_str = str;
	else
		np->n_un.n_val = val;
	return (np);
}

/*
 * Build an execution node
 * for -ok or -exec.
 */
NODE *enode(int type)
{
	register NODE *np;
	register char **app;
	register char *ap;

	seflag++;
	np = snode(xexec, NULL);
	np->n_type = type;
	if ((np->n_un.n_strp = (char**)malloc(sizeof(char*[NARG])))==NULL)
		errx(1, nospace);
	app = np->n_un.n_strp;
	for (;;) {
		if ((ap = next()) == NULL)
			errx(1, "Non-terminated -exec or -ok command list");
		if (strcmp(ap, "{}") == 0)
			ap = FILEARG;
		else if (strcmp(ap, ";") == 0)
			break;
		if (app-np->n_un.n_strp >= NARG-1)
			errx(1, "Too many -exec or -ok command arguments");
		*app++ = ap;
	}
	*app = NULL;
	return (np);
}

/*
 * Execute find on a single
 * pathname hierarchy.
 */

void find(char *dir)
{
	register char *ep, *cp;

	cp = dir;
	ep = fname;
	while (*cp)
		*ep++ = *cp++;
	*ep = '\0';
	if (stat(dir, &sb) < 0)
		err(1, "Cannot find directory `%s'", dir);
	if ((sb.st_mode&S_IFMT) != S_IFDIR)
		errx(1, "%s: not a directory", dir);
	fentry(ep, &sb);
}

/*
 * The pointer is the end pointer
 * into the fname buffer.
 * And the stat buffer is passed to this
 * which traverses the directory hierarchy.
 */
void fentry(char *ep, struct stat *sbp)
{
	register char *np;
	struct dirent *dp;
	DIR *dir;
	register int nb;
	int fd;
	int dirflag;
	char *iobuf;

	if (sbp != NULL) {
		dirflag = (sbp->st_mode&S_IFMT)==S_IFDIR;
		execute(code);
	} else
		dirflag = 1;
	if (dirflag) {
		if (++depth >= NRECUR) {
			depth = 0;
			ffork(ep, sbp);
			return;
		}
		if ((dir = opendir(fname)) == NULL) {
			warn("%s: cannot open directory", fname);
			return;
		}
		while ((dp = readdir(dir)) != NULL) {
			if (dp->d_ino == 0)
				continue;
			np = dp->d_name;
			if (*np++=='.'
			  && (*np=='\0' || (*np++=='.' && *np=='\0')))
				continue;
			np = buildname(dp, ep);
			if (stat(fname, &sb) < 0) {
				warn("%s: cannot stat", fname);
				continue;
			}
			fentry(np, &sb);
		}
		*ep = '\0';
		closedir(dir);
		depth--;
	}
}

/*
 * Fork to do a find on recursive directory
 * structure that is too deep to fit into
 * user's open files.
 */
void ffork(char *ep, struct stat *sbp)
{
	register int i;
	register int pid;
	int status;
	int nfile = sysconf(_SC_OPEN_MAX);

	fflush(stdout);
	if ((pid = fork()) < 0) {
		warn(toodeep);
		return;
	}
	if (pid) {
		while (wait(&status) >= 0)
			;
		if (status)
			warn("panic: child failed: %o", status);
		return;
	}
	for (i=3; i<nfile; i++)
		close(i);
	fentry(ep, (struct stat *)NULL);
	fflush(stdout);
	exit(0);
}

/*
 * Build up the next entry
 * in the name.
 */
char *buildname(struct dirent *dp, char *ep)
{
	register char *cp = dp->d_name;
	register unsigned n = DIRSIZ;

	if (ep+DIRSIZ+2 > &fname[NFNAME-1]) {
		warn(toodeep);
		return (NULL);
	}
	if (ep>fname && ep[-1]!='/')
		*ep++ = '/';
	do {
		if (*cp == '\0')
			break;
		*ep++ = *cp++;
	} while (--n);
	*ep = '\0';
	return (ep);
}

/*
 * Execute compiled code.
 */
int execute(NODE *np)
{
	switch (np->n_op) {
	case AND:
		if (execute(np->n_left) && execute(np->n_right))
			return (1);
		return (0);

	case OR:
		if (execute(np->n_left) || execute(np->n_right))
			return (1);
		return (0);

	case '!':
		return (!execute(np->n_left));

	case FUN:
		return ((*np->n_fun)(np));

	default:
		errx(1, "Panic: bad expression tree (op %d)", np->n_op);
	}
	/* NOTREACHED */
}

/* 
 * pnmatch(string, pattern, unanchored)
 * returns 1 if pattern matches in string.
 * pattern:
 *	[c1c2...cn-cm]	class of characters.
 *	?		any character.
 *	*		any # of any character.
 *	^		beginning of string (if unanchored)
 *	$		end of string (if unanchored)
 * unanch:
 *	0		normal (anchored) pattern.
 *	1		unanchored (^$ also metacharacters)
 *	>1		end unanchored.
 * >1 is used internally but should not be used by the user.
 */

int pnmatch(const char *s, const char *p, int unanch)
{
	int c1;
	int c2;

	if (unanch == 1) {
		while (*s)
			if (pnmatch(s++, p, ++unanch))
				return (1);
		return (0);
	}
	while (c2 = *p++) {
		c1 = *s++;
		switch(c2) {
		case '^':
			if (unanch == 2) {
				s--;
				continue;
			} else if (unanch == 0)
				break;
			else
				return (0);

		case '$':
			if (unanch)
				return (c1 == '\0');
			break;

		case '[':
			for (;;) {
				c2 = *p++;
				if (c2=='\0' || c2==']')
					return (0);
				if (c2 == '\\' && *p == '-') 
					c2 = *p++;
				if (c2 == c1)
					break;
				if (*p == '-')
					if (c1<=*++p && c1>=c2)
						break;
			}
			while (*p && *p++!=']')
				;

		case '?':
			if (c1)
				continue;
			return(0);

		case '*':
			if (!*p)
				return(1);
			s--;
			do {
				if (pnmatch(s, p, unanch))
					return (1);
			} while(*s++ != '\0');
			return(0);

		case '\\':
			if ((c2 = *p++) == '\0')
				return (0);
		}
		if (c1 != c2)
			return (0);
	}
	return(unanch ? 1 : !*s);
}

/*
 * Check for a match on the filename
 */
int xname(NODE *np)
{
	register char *ep;

	ep = fname;
	while (*ep != '\0')
		ep++;
	while (ep>fname && *--ep!='/')
		;
	if (*ep == '/')
		ep++;
	return (pnmatch(ep, np->n_un.n_str, 0));
}

/*
 * Compare the mode for a match again
 * octal number `np->n_un.n_val'.
 */
int xperm(NODE *np)
{
	register int onum;
	register int mode;

	mode = np->n_type<0 ? sb.st_mode&017777 : sb.st_mode&0777;
	onum = np->n_un.n_val;
	if (np->n_type < 0)
		return ((mode&onum) == onum);
	return (mode == onum);
}

/*
 * Compare again filetypes
 */
int xtype(NODE *np)
{
	register char *type;
	register int ftype;

	type = np->n_un.n_str;
	ftype = sb.st_mode&S_IFMT;
	if (type[1] == '\0')
		switch (type[0]) {
		case 'b':
			return (ftype == S_IFBLK);
	
		case 'c':
			return (ftype == S_IFCHR);
	
		case 'd':
			return (ftype == S_IFDIR);
	
		case 'f':
			return (ftype == S_IFREG);

#ifdef S_IFMPB	
		case 'm':
			return (ftype==S_IFMPB || ftype==S_IFMPC);
#endif			
		case 'p':
			return (ftype == S_IFIFO);
		}
	errx(1, "Bad file type `%s'", type);
}

/*
 * Numerical compare.
 */
int ncomp(NODE *np, unsigned int val)
{
	if (np->n_type == 0)
		return (np->n_un.n_val == val);
	if (np->n_type > 0)
		return (val > np->n_un.n_val);
	return (val < np->n_un.n_val);
}
/*
 * Compare link counts.
 */
int xlinks(NODE *np)
{
	return (ncomp(np, sb.st_nlink));
}

/*
 * Compare uid.
 */
int xuser(NODE *np)
{
	return (np->n_un.n_val == sb.st_uid);
}

/*
 * Compare group id of file
 * with given one.
 */
int xgroup(NODE *np)
{
	return (np->n_un.n_val == sb.st_gid);
}

/*
 * Compare size of file in blocks
 * with given.
 */
int xsize(NODE *np)
{
	register int fsize;

	fsize = (sb.st_size+BUFSIZ-1)/BUFSIZ;
	return (ncomp(np, fsize));
}

/*
 * Compare the i-number of the file
 * with that given.
 */
int xinum(NODE *np)
{
	return (ncomp(np, sb.st_ino));
}

/*
 * Do a numerical comparison on dates.
 */
int ndays(NODE *np, time_t t)
{
	register int days;

	days = (curtime-t+DAYSEC/2)/DAYSEC;
	return (ncomp(np, days));
}


/*
 * Return true if file has been accessed
 * in `n' days.
 */
int xatime(NODE *np)
{
	return (ndays(np, sb.st_atime));
}

/*
 * Return non-zero if file has been created
 * in `n' days.
 */
int xctime(NODE *np)
{
	return (ndays(np, sb.st_ctime));
}

/*
 * Return true if file has been modified
 * in `n' days.
 */
int xmtime(NODE *np)
{
	return (ndays(np, sb.st_mtime));
}

/*
 * Execute a command based on the filename
 */
int xexec(NODE *np)
{
	static char command[200];
	register char *ap, **app;
	register int c;
	int ok;

	command[0] = '\0';
	app = np->n_un.n_strp;
	while (*app != NULL) {
		if ((ap = *app++) == FILEARG)
			ap = fname;
		strcat(command, ap);
		if (*app != NULL)
			strcat(command, " ");
	}
	if (np->n_type) {
		printf("%s%s? ", prompt, command);
		ok = (c = getchar()) == 'y';
		while (c!='\n' && c!=EOF)
			c = getchar();
		if (!ok)
			return (0);
	}
	return (!system(command));
}

/*
 * Print the filename.
 */
/* ARGSUSED */
int xprint(NODE *np)
{
	printf("%s\n", fname);
	return (1);
}

int xnop(NODE *np)
{
	return (1);
}

/*
 * Return true if the file is newer than
 * the given one.
 */
int xnewer(NODE *np)
{
	return (sb.st_mtime > np->n_un.n_time);
}

void usage(void)
{
	fprintf(stderr, "Usage: find directory ... [ expression ]\n");
}
