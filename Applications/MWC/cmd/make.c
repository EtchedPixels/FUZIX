/*
 * make -- maintain program groups
 * td 80.09.17
 * things done:
 *	20-Oct-82	Made nextc properly translate "\\\n[ 	]*" to ' '.
 *	15-Jan-85	Made portable enough for z-8000, readable enough for
 *			human beings.
 *	06-Nov-85	Added free(t) to make() to free used space.
 *	07-Nov-85	Modified docmd() to call varexp() only if 'cmd'
 *			actually contains macros, for efficiency.
 *	24-Feb-86	Minor fixes by rec to many things.  Deferred
 *			macro expansion in actions until needed, deferred
 *			getmdate() until needed, added canonicalization in
 *			archive searches, allowed ${NAME} in actions for
 *			shell to expand, put macro definitions in malloc,
 *			entered environ into macros.
 *	17-Oct-86	Very minor MS-DOS changes by steve: add _cmdname[],
 *			conditionalize archive code as #if COHERENT || GEMDOS.
 *	 8-Dec-86	Rec makes inpath() search current directory first,
 *			and allows : in dependency list under MSDOS && GEMDOS.
 *	 8-Feb-91	steve: fix comment handling, allow "\#", allow ${VAR}.
 *			Add docmd0() to make $< and $* work in Makefiles.
 *	12-Feb-91	steve: add $SRCPATH source path searching.
 *	 1-Nov-91	steve: fix bug in nextc() to handle "\n\t\n" correctly
 *      29-Sep-92	michael: fix problem with defining a rule that also
 *				exists in the ACTIONFILE.	
 *      08-Oct-92	michael: fix problem with making targets with no
 *				specified actions (empty productions).
 */

#include	"make.h"

char usage[] = "Usage: make [-deinpqrst] [-f file] [macro=value] [target]";
char nospace[] = "out of space";
char badmac[] = "bad macro name";
char incomp[] = "incomplete line at end of file";

/* Command line flags. */
int iflag;			/* ignore command errors */
int sflag;			/* don't print command lines */
int rflag;			/* don't read built-in rules */
int nflag;			/* don't execute commands */
int tflag;			/* touch files rather than run commands */
int qflag;			/* zero exit status if file up to date */
int pflag;			/* print macro defns, target descriptions */
int dflag;			/* debug mode -- verbose printout */
int eflag;			/* make environ macros protected */

/* Globals. */
unsigned char	backup[NBACKUP];
int		defining;	/* nonzero => do not expand macros */
int		instring;	/* Are we in the middle of a string? */
SYM		*deflt;
char		*deftarget;
FILE		*fd;
int		lastc;
int		lineno;
MACRO		*macro;
char		macroname[NMACRONAME+1];
char		*mvarval[4];		/* list of macro variable values */
int		nbackup;
time_t		now;
char		*srcpath;
struct stat	statbuf;
SYM		*suffixes;
SYM		*sym;
char		tokbuf[NTOKBUF];
char		*token;
int		toklen;
char		*tokp;
int 		inactionfile = 0;

/* cry and die */
/* VARARGS */
void die(const char *s, ...)
{
	va_list args;
	va_start(args, s);
	fflush(stdout);
	verrx(ERROR, "make: ", args);
	exit(ERROR);
}

/* print lineno, cry and die */
/* VARARGS */
void doerr(const char *s, ...)
{
	va_list args;
	va_start(args, s);
	fprintf(stderr, "make: %d: ", lineno);
	vfprintf(stderr, s, args);
	exit(ERROR);
}

/* Malloc nbytes and abort on failure */
char *mmalloc(size_t n)
{
	char *p;
	if (p = malloc(n))
		return p;
	doerr(nospace);
}

/* Whine about usage and then quit */
void Usage(void)
{
	fprintf(stderr, "%s\n", usage);
	exit(1);
}

/* read characters from backup (where macros have been expanded) or from
 * whatever the open file of the moment is. keep track of line #s.
 */
int readc(void)
{
	if (nbackup!=0)
		return(backup[--nbackup]);
	if (lastc=='\n')
		lineno++;
	lastc=getc(fd);
	return(lastc);
}

/* put c into backup[] */
void putback(int c)
{
	if (c==EOF)
		return;
	if (nbackup == NBACKUP)
		doerr("macro definition too long");
	backup[nbackup++]=c;
}

/* put s into backup */
void unreads(register char *s)
{
	register char *t;

	t = &s[strlen(s)];
	while (t > s)
		putback(*--t);
}

/* return a pointer to the macro definition assigned to macro name s.
 * return NULL if the macro has not been defined.
 */
char *mexists(register char *s)
{
	register MACRO *i;

	for (i = macro; i != NULL; i=i->next)
		if (Streq(s, i->name))
			return (i->value);

	return (NULL);
}

/* install macro with name name and value value in macro[]. Overwrite an
 * existing value if it is not protected.
 */
void define(char *name, char *value, int protected)
{
	register MACRO *i;

	if (dflag)
		printf("define %s = %s\n", name, value);
	for (i = macro; i != NULL; i=i->next)
		if (Streq(name, i->name)) {
			if (!i->protected) {
				free(i->value);
				i->value = value;
				i->protected = protected;
			} else if (dflag)
				printf("... definition suppressed\n");
			return;
		}
	i = (MACRO *)mmalloc(sizeof(*i));
	i->name = name;
	i->value = value;
	i->protected = protected;
	i->next = macro;
	macro = i;
}

/* Accept single letter user defined macros */
int ismacro(int c)
{
	return ((c>='0'&&c<='9')
		|| (c>='a'&&c<='z')
		|| (c>='A'&&c<='Z'));
}

/*
 * Return the next character from the input file.
 * Eat comments.
 * Return EOS for newline not followed by an action.
 * Return '\n' for newline followed by an action.
 * If not in a macro definition or action specification,
 * then expand macro in backup or complain about the name.
 */
int nextc(void)
{
	register char *s;
	register int c, endc;

Again:
	if ((c = readc()) == '\\') {
		c = readc();
		if (c == '\n') {		/* escaped newline */
			while ((c=readc())==' ' || c=='\t')
				;		/* eat following whitespace */
			putback(c);
			return(' ');
		} else if (c == '#')
			return c;		/* "\#" means literal '#' */
		putback(c);
		return '\\';
	}
	if ((c=='#') && !instring) {
		do
			c = readc();
		while (c != '\n' && c != EOF);
	}
	if ((c=='"') || (c=='\''))
	{
		instring = !instring;
		return c;
	}
	if (c == '\n') {
		instring = 0;
Again2:
		if ((c = readc()) != ' ' && c != '\t') {
			putback(c);
			if (c == '#')
				goto Again;	/* "\n# comment" */
			return EOS;		/* no action follows */
		}
		do
			c = readc();
		while (c == ' ' || c == '\t');	/* skip whitespace */
		if (c == '\n')
			goto Again2;		/* "\n\t\n" */
		putback(c);
		if (c == '#')
			goto Again;		/* "\n\t# comment" */
		return '\n';			/* action follows */
	}
	if (!defining && c=='$'){
		c=readc();
		if (c == '(' || c == '{') {
			endc = (c == '(') ? ')' : '}';
			s = macroname;
			while (' ' < (c = readc()) && c < 0177 && c != endc)
				if (s != &macroname[NMACRONAME])
					*s++=c;
			if (c != endc)
				doerr(badmac);
			*s++ = '\0';
		} else if (ismacro(c)) {
			macroname[0]=c;
			macroname[1]='\0';
		} else
			doerr(badmac);
		if ((s=mexists(macroname))!=NULL)
			unreads(s);
		goto Again;
	}

	return(c);
}

/* Get a block of l0+l1 bytes copy s0 and s1 into it, and return a pointer to
 * the beginning of the block.
 */
char *extend(char *s0, int l0, char *s1, int l1)
{
	register char *t;

 	if (s0 == NULL)
 		t = mmalloc(l1);
 	else {
 		if ((t = realloc(s0, l0 + l1)) == NULL)
			doerr(nospace);
	}
	strncpy(t+l0, s1, l1);
	return(t);
}

/* Return 1 if c is EOS, EOF, or one of the characters in s */
int delim(char c, char *s)
{
	return (c == EOS || c == EOF || index(s, c) != NULL);
}

/* Prepare to copy a new token string into the token buffer; if the old value
 * in token wasn't saved, tough matzohs.
 */
void starttoken(void)
{
	token=NULL;
	tokp=tokbuf;
	toklen=0;
}

/* Put c in the token buffer; if the buffer is full, copy its contents into
 * token and start agin at the beginning of the buffer.
 */
void addtoken(int c)
{
	if (tokp==&tokbuf[NTOKBUF]){
		token=extend(token, toklen-NTOKBUF, tokbuf, NTOKBUF);
		tokp=tokbuf;
	}
	*tokp++=c;
	toklen++;
}

/* mark the end of the token in the buffer and save it in token. */
void endtoken(void)
{
	addtoken('\0');
	token=extend(token, toklen-(tokp-tokbuf), tokbuf, tokp-tokbuf);
}

/* Install value at the end of the token list which begins with next; return
 * a pointer to the beginning of the list, which is the one just installed if
 * next was NULL.
 */
TOKEN *listtoken(char *value, TOKEN *next)
{
	register TOKEN *p;
	register TOKEN *t;

	t=(TOKEN *)mmalloc(sizeof *t);	/*Necessaire ou le contraire?*/
	t->value=value;
	t->next=NULL;
	if (next==NULL)
		return(t);
	for(p=next;p->next!=NULL;p=p->next);
	p->next=t;
	return(next);
}

/* Free the overhead of a token list */
TOKEN *freetoken(register TOKEN *t)
{
	register TOKEN *tp;
	while (t != NULL) {
		tp = t->next;
		free(t);
		t = tp;
	}
	return t;
}

/* Read macros, dependencies, and actions from the file with name file, or
 * from whatever file is already open. The first string of tokens is saved
 * in a list pointed to by tp; if it was a macro, the definition goes in
 * token, and we install it in macro[]; if tp points to a string of targets,
 * its depedencies go in a list pointed to by dp, and the action to recreate
 * it in token, and the whole shmear is installed.
 */
void input(const char *file)
{
	TOKEN *tp = NULL, *dp = NULL;
	register int c;
	char *action;
	int twocolons;

	if (file!=NULL && (fd=fopen(file, "r"))==NULL)
		die("cannot open %s", file);
	lineno=1;
	lastc=EOF;
	for(;;){
		c=nextc();
		for(;;){
			while(c==' ' || c=='\t')
				c=nextc();
			if (delim(c, "=:;\n"))
				break;
			starttoken();
			while(!delim(c, " \t\n=:;")){
				addtoken(c);
				c=nextc();
			}
			endtoken();
			tp=listtoken(token, tp);
		}
		switch(c){
		case EOF:
			if (tp!=NULL)
				doerr(incomp);
			fclose(fd);
			return;
		case EOS:
			if (tp==NULL)
				break;
		case '\n':
			doerr("newline after target or macroname");
		case ';':
			doerr("; after target or macroname");
		case '=':
			if (tp==NULL || tp->next!=NULL)
				doerr("= without macro name or in token list");
			defining++;
			while((c=nextc())==' ' || c=='\t');
			starttoken();
			while(c!=EOS && c!=EOF) {
				addtoken(c);
				c=nextc();
			}
			endtoken();
			define(tp->value, token, 0);
			defining=0;
			break;
		case ':':
			if (tp==NULL)
				doerr(": without preceding target");
			c=nextc();
			if (c==':'){
				twocolons=1;
				c=nextc();
			} else
				twocolons=0;
			for(;;){
				while(c==' ' || c=='\t')
					c=nextc();
				if (delim(c, "=:;\n"))
					break;
				starttoken();
				while(!delim(c, TDELIM)){
					addtoken(c);
					c=nextc();
				}
				endtoken();
				dp=listtoken(token, dp);
			}
			switch(c){
			case ':':
				doerr("::: or : in or after dependency list");
			case '=':
				doerr("= in or after dependency");
			case EOF:
				doerr(incomp);
			case ';':
			case '\n':
				++defining;
				starttoken();
				while((c=nextc())!=EOS && c!=EOF)
					addtoken(c);
				endtoken();
				defining = 0;
				action=token;
				break;
			case EOS:
				action=NULL;
			}
			install(tp, dp, action, twocolons);
		}
		tp = freetoken(tp);
		dp = freetoken(dp);
		dp = NULL;
	}
}

char *path(const char *p1, const char *p2, int mode)
{
	static char *pv = NULL;
	int l1 = strlen(p1);
	if (pv)
		free(pv);
	pv = malloc(strlen(p1) + strlen(p2) + 2);
	strcpy(pv, p1);
	pv[l1] = '/';
	strcpy(pv + l1 + 1, p2);
	if (access(pv, mode) == 0)
		return pv;
	free(pv);
	return NULL;
}

/* Input with library lookup */
void inlib(const char *file)
{
	const char *p, *cp;
	if ((p = getenv("LIBPATH")) == NULL)
		p = "/lib:/usr/lib";
	cp = path(p, file, R_OK);
	input(cp ? cp : file);
}

/* Input first file in list which is found via SRCPATH. */
/* Look in current directory first */
void inpath(char *file, ...)
{
	register char **vp, *cp;
	va_list ap;
	va_start(ap, file);

	cp = NULL;
	for (vp = va_arg(ap, char **); *vp != NULL;)
		if (access(*vp, R_OK) >= 0) {
			cp = *vp;
			break;
		}
	va_end(ap);
	if ( ! cp) {
		va_start(ap, file);
		for (vp = va_arg(ap, char **); *vp != NULL; vp += 1)
			if ((cp = path(srcpath, *vp, R_OK)) != NULL)
				break;
		va_end(ap);
	}
	input(cp ? cp : file);
}

/* Return the last modified date of file with name name. If it's an archive,
 * open it up and read the insertion date of the pertinent member.
 */
time_t getmdate(char *name)
{
#if	_I386
	char	*subname;
	char	*lwa;
	int	fd, x;
	char	magic[SARMAG];
	int	size;

	time_t	result;
	struct ar_hdr	hdrbuf;
#endif

	if (stat(name, &statbuf) == 0)
		return(statbuf.st_mtime);


#if	_I386
	subname = index(name, '(');
	if (subname == NULL)
		return (0);
	lwa = &name[strlen(name) - 1];
	if (*lwa != ')')
		return (0);
	*subname = NUL;
	fd = open(name, READ);
	*subname++ = '(';
	if (fd == EOF)
		return (0);
	if (read(fd, magic, SARMAG) != SARMAG)
	{
		close(fd);
		return (0);
	}
	if (!strcmp(magic, ARMAG)) {
		close(fd);
		return (0);
	}
	*lwa = NUL;
	result = 0;
	while (read(fd, &hdrbuf, sizeof hdrbuf) == sizeof hdrbuf) {
		if ((strncmp(hdrbuf.ar_name, subname, x = strlen(subname)) == 0)
		    && (hdrbuf.ar_name[x] == '/'))
		{
			result = atoi(hdrbuf.ar_date);
			break;
		}
		size = atoi(hdrbuf.ar_size);
		lseek(fd, size, SEEK_CUR);
	}
	*lwa = ')';

	return (result);
#else
	return 0;
#endif
}


/* Does file name exist? */
int fexists(char *name)
{
#if 0
	if (dflag)
		printf("fexists(%s) = %d getmdate(name) = %d\n", name,
		getmdate(name) != 0, getmdate(name));
#endif
	return getmdate(name) != 0;
}

/*
 * Find name on srcpath.
 * Return 'name' unchanged if file exists as 'name', 'name' is absolute,
 * or 'name' not found on sourcepath.
 * If successful, return pointer to allocated copy.
 */
char *fpath(char *name)
{
	register char *s;

	if (fexists(name)
	 || *name == '/'
	 || srcpath == NULL
	 || (s = path(srcpath, name, R_OK)) == NULL)
		return name;
	starttoken();
	while (*s)
		addtoken(*s++);
	endtoken();
	return token;
}

/* Return a pointer to the symbol table entry with name "name", NULL if it's
 * not there.
 */
SYM *sexists(char *name)
{
	register SYM *sp;

	for(sp=sym;sp!=NULL;sp=sp->next)
		if (Streq(name, sp->name))
			return(sp);
	return(NULL);
}

/*
 * Return a pointer to the member of deplist which has name as the last
 * part of it's pathname, otherwise return NULL.
 */
SYM *dexists(char *name, DEP *dp)
{
	register char *p;
	while (dp != NULL) {
		if ((p = rindex(dp->symbol->name, '/')) && Streq(name, p+1))
			return dp->symbol;
		else
			dp = dp->next;
	}
	return NULL;
}

/* Look for symbol with name "name" in the symbol table; install it if it's
 * not there; initialize the action and dependency lists to NULL, the type to
 * unknown, zero the modification date, and return a pointer to the entry.
 */
SYM *lookup(char *name)
{
	register SYM *sp;

	if ((sp=sexists(name))!=NULL)
		return(sp);
	sp = (SYM *)mmalloc(sizeof (*sp));	/*necessary?*/
	sp->name=name;
	sp->filename=fpath(name);
	sp->action=NULL;
	sp->deplist=NULL;
	sp->type=T_UNKNOWN;
	sp->moddate=0;
	sp->next=sym;
	sym=sp;
	return(sp);
}

/* Install a dependency with symbol having name "name", action "action" in
 * the end of the dependency list pointed to by next. If s has already
 * been noted as a file in the dependency list, install action. Return a
 * pointer to the beginning of the dependency list.
 */
DEP *adddep(char *name, char *action, DEP *next)
{
	register DEP *v;
	register SYM *s;
	DEP *dp;

	s=lookup(name);
	for(v=next;v!=NULL;v=v->next)
		if (s==v->symbol){
			if (action != NULL) {
				if (v->action!=NULL)
					doerr("multiple detailed actions for %s",
						s->name);
				v->action=action;
			}
			return(next);
		}
	v = (DEP *)malloc(sizeof (*v));	/*necessary?*/
	v->symbol=s;
	v->action=action;
	v->next=NULL;
	if (next==NULL)
		return(v);
	for(dp=next;dp->next!=NULL;dp=dp->next);
	dp->next=v;
	return(next);
}

/* Do everything for a dependency with left-hand side cons, r.h.s. ante,
 * action "action", and one or two colons. If cons is the first target in the
 * file, it becomes the default target. Mark each target in cons as detailed
 * if twocolons, undetailed if not, and install action in the symbol table
 * action slot for cons in the latter case. Call adddep() to actually create
 * the dependency list.
 */

void install(TOKEN *cons, TOKEN *ante, char *action, int twocolons)
{
	SYM *cp;
	TOKEN *ap;

	if (deftarget==NULL && cons->value[0]!='.')
		deftarget=cons->value;
	if (dflag){
		printf("Ante:");
		ap=ante;
		while(ap!=NULL){
			printf(" %s", ap->value);
			ap=ap->next;
		}
		printf("\nCons:");
		ap=cons;
		while(ap!=NULL){
			printf(" %s", ap->value);
			ap=ap->next;
		}
		printf("\n");
		if (action!=NULL)
			printf("Action: '%s'\n", action);
		if (twocolons)
			printf("two colons\n");
	}
	for (; cons != NULL; cons = cons->next) {
		cp=lookup(cons->value);
		if (cp==suffixes && ante==NULL)
			cp->deplist=NULL;
		else{
			if (twocolons){
				if (cp->type==T_UNKNOWN)
					cp->type=T_DETAIL;
				else if (cp->type!=T_DETAIL)
					doerr("'::' not allowed for %s",
						cp->name);
			} else {
				if (cp->type==T_UNKNOWN)
					cp->type=T_NODETAIL;
				else if (cp->type!=T_NODETAIL)
					doerr("must use '::' for %s", cp->name);
				if (action != NULL) {
					if (cp->action != NULL)
					{
						if (!inactionfile)
							doerr("multiple action"
							"s for %s", cp->name);
					}
					else
						cp->action = action;
				}
			}
			for(ap=ante;ap!=NULL;ap=ap->next)
				cp->deplist=adddep(ap->value,
					twocolons?action:NULL, cp->deplist);
		}
	}
}

/* Make s; first, make everything s depends on; if the target has detailed
 * actions, execute any implicit actions associated with it, then execute
 * the actions associated with the dependencies which are newer than s.
 * Otherwise, put the dependencies that are newer than s in token ($?),
 * make s if it doesn't exist, and call docmd.
 */
void make(SYM *s)
{
	register DEP *dep;
	register char *t, *name;
	int update;
	int type;

	if (s->type==T_DONE)
		return;
	name = s->filename;
	if (dflag) {
		if (s->name == name)
			printf("Making %s\n", name);
		else
			printf("Making %s (file %s)\n", s->name, name);
	}
	type=s->type;
	s->type=T_DONE;
	s->moddate=getmdate(name);
	for(dep=s->deplist;dep!=NULL;dep=dep->next)		
		make(dep->symbol);
	if (type==T_DETAIL){
		implicit(s, "", 0);
		for(dep=s->deplist;dep!=NULL;dep=dep->next)
			if (dep->symbol->moddate>s->moddate)
				docmd0(s, dep->action, name, dep->symbol->filename);
	} else {
		update=0;
		starttoken();
		for(dep=s->deplist;dep!=NULL;dep=dep->next){
			if (dflag)
				printf("%s time=%ld %s time=%ld\n",
				    dep->symbol->filename, dep->symbol->moddate,
				    name, s->moddate);
			if (dep->symbol->moddate>s->moddate){
				update++;
				addtoken(' ');
				for(t=dep->symbol->filename;*t;t++)
					addtoken(*t);
			}
		}
		endtoken();
		t = token;
		if (!update && !fexists(name)) {
			update = TRUE;
			if (dflag)
				printf("'%s' made due to non-existence\n",
					name);
		}
		if (s->action==NULL)
			implicit(s, t, update);
		else if (update)
			docmd0(s, s->action, name, t);
		free(t);
	}
}

/*
 * Expand substitutes the macros in actions and returns the string.
 */
void expand(char *str)
{
	register int c;
	register char *p;
	int endc;

	while (c = *str++) {
		if (c == '$') {
			c = *str++;
			switch (c) {
			case '\0':	doerr(badmac);
			case '$':	addtoken(c);	continue;
			case '@':	p = mvarval[0]; break;
			case '?':	p = mvarval[1]; break;
			case '<':	p = mvarval[2]; break;
			case '*':	p = mvarval[3]; break;
			case '{':
			case '(':
				endc = (c == '(') ? ')' : '}';
				c = '(';
				p = str;
				do c = *str++; while (c != 0 && c != endc);
				if (c == 0)
					doerr(badmac);
				*--str = 0;
				p = mexists(p);
				*str++ = endc;
				break;
			default:
				if ( ! ismacro(c))
					doerr(badmac);
				c = *str;
				*str = 0;
				p = mexists(str-1);
				*str = c;
				break;
			}
			if (p != NULL)
				expand(p);
		} else
			addtoken(c);
	}
}

/* Like docmd(), except builds its own dependency list and prefix args. */
void docmd0(SYM *s, char *cmd, char *at, char *ques)
{
	register char *cp;
	register DEP *dep;
	char *less, *prefix;

	/* Build dependency list. */
	starttoken();
	for (dep = s->deplist; dep != NULL; dep = dep->next) {
		addtoken(' ');
		for (cp = dep->symbol->filename; *cp; cp++)
			addtoken(*cp);
	}
	endtoken();
	less = token;

	/* Build prefix. */
	starttoken();
	for (cp = s->name; *cp; cp++)
		addtoken(*cp);
	endtoken();
	prefix = token;

	if ((cp = rindex(prefix, '.')) != NULL)
		*cp = '\0';
	docmd(s, cmd, at, ques, less, prefix);
	free(less);
	free(prefix);
}

/* Mark s as modified; if tflag, touch s, otherwise execute the necessary
 * commands.
 */
void docmd(SYM *s, char *cmd, char *at, char *ques, char *less, char *star)
{
	if (dflag)
		printf("ex '%s'\n\t$@='%s'\n\t$?='%s'\n\t$<='%s'\n\t$*='%s'\n",
			cmd, at, ques, less, star);
	if (qflag)
		exit(NOTUTD);
	s->moddate = now;
	if (tflag)
		cmd = "touch $@";
	if (cmd == NULL)
		return;
	mvarval[0] = at;
	mvarval[1] = ques;
	mvarval[2] = less;
	mvarval[3] = star;
	starttoken();
	expand(cmd);
	endtoken();
	doit(token);
	free(token);
}


/* look for '-' (ignore errors) and '@' (silent) in cmd, then execute it
 * and note the return status.
 */
void doit(char *cmd)
{
	register char *mark;
	int sflg, iflg, rstat;

	if (nflag) {
		printf("%s\n", cmd);
		return;
	}
	do {
		mark = index(cmd, '\n');
		if (mark != NULL)
			*mark = NUL;
		if (*cmd == '-') {
			++cmd;
			iflg = TRUE;
		} else
			iflg = iflag;
		if (*cmd == '@') {
			++cmd;
			sflg = TRUE;
		} else
			sflg = sflag;
		if (!sflg)
			printf("%s\n", cmd);
		fflush(stdout);
		rstat = system(cmd);
		if (rstat != 0 && !iflg)
			if (sflg)
				die("%s	exited with status %d",
					cmd, rstat);
			else
				die("	exited with status %d", rstat);
		cmd = mark + 1;
	} while (mark != NULL && *cmd != NUL);
}


/* Find the implicit rule to generate obj and execute it. Put the name of
 * obj up to '.' in prefix, and look for the rest in the dependency list
 * of .SUFFIXES. Find the file "prefix.foo" upon which obj depends, where
 * foo appears in the dependency list of suffixes after the suffix of obj.
 * Then make obj according to the rule from makeactions. If we can't find
 * any rules, use .DEFAULT, provided we're definite.
 */

void implicit(SYM *obj, char *ques, int definite)
{
	register char *s;
	register DEP *d;
	char *prefix, *file, *rulename, *suffix;
	SYM *rule;
	SYM *subj;

	if (dflag)
		printf("Implicit %s (%s)\n", obj->name, ques);
	if ((suffix=rindex(obj->name, '.')) == NULL
	 || suffix==obj->name) {
		if (definite)
			defalt(obj, ques);
		return;
	}
	starttoken();
	for(s=obj->name; s<suffix; s++)
		addtoken(*s);
	endtoken();
	prefix=token;
	for(d=suffixes->deplist;d!=NULL;d=d->next)
		if (Streq(suffix, d->symbol->name))
			break;
	if (d==NULL){
		free(prefix);
		if (definite)
			defalt(obj, ques);
		return;
	}
	while((d=d->next)!=NULL){
		starttoken();
		for(s=obj->name; s!=suffix; s++)
			addtoken(*s);
		for(s=d->symbol->name;*s;s++)
			addtoken(*s);
		endtoken();
		file=token;
		if ((s = fpath(file)) != file) {
			free(file);
			file = s;
		}
		subj=NULL;
		if (fexists(file) || (subj=dexists(file, obj->deplist))){
			starttoken();
			for(s=d->symbol->filename;*s!='\0';s++)
				addtoken(*s);
			for(s=suffix;*s!='\0';s++)
				addtoken(*s);
			endtoken();
			rulename=token;
			if ((rule=sexists(rulename))!=NULL){
				if (subj != NULL || (subj=sexists(file))) {
					free(file);
					file=subj->name;
				} else
					subj=lookup(file);
				make(subj);
				if (definite || subj->moddate>obj->moddate)
					docmd(obj, rule->action,
						obj->name, ques, file, prefix);
				free(prefix);
				free(rulename);
				return;
			}
			free(rulename);
		}
		free(file);
	}
	free(prefix);
	if (definite)
		defalt(obj, ques);
}

/*
 * Deflt uses the commands associated to '.DEFAULT' to make the object
 * 'obj'.
 */

void defalt(SYM *obj, char *ques)
{
	if (deflt == NULL)
	{
		if (obj->deplist == NULL)
			die("do not know how to make %s", obj->name);
	}
	else
		docmd0(obj, deflt->action, obj->name, ques);
}

int main(int argc, char *argv[])
{
	register char	*s, *value;
	register char	*namesave;
	register int c;
	int	len, numtargets = 0;
	char 	**dtarget;
	TOKEN	*fp = NULL;
	SYM	*sp;
	DEP	*d;
	MACRO	*mp;
	extern char **environ;
	char	**envp = environ;


	if ((dtarget = malloc(argc * sizeof(char *))) == NULL)
		doerr(nospace);

	time(&now);
	++argv;
	--argc;

	while (argc > 0)
	{
		if (argv[0][0] == '-')
		{
			for (--argc, s = *argv++; *++s != NUL;)
				switch (*s) {
				case 'd': dflag++; break;
				case 'e': eflag++; break;
				case 'i': iflag++; break;
				case 'n': nflag++; break;
				case 'p': pflag++; break;
				case 'q': qflag++; break;
				case 'r': rflag++; break;
				case 's': sflag++; break;
				case 't': tflag++; break;
				case 'f':
					if (--argc < 0)
						Usage();
					fp=listtoken(*argv++, fp);
					break;
				default:
					Usage();
				}
		}
		else if ((value = index(*argv, '=')) != NULL)
		{
			s = *argv;
			while (*s != ' ' && *s != '\t' && *s != '=')
				++s;
			*s = '\0';
			define(*argv++, value+1, 1);
			--argc;
		}
		else
		{
			dtarget[numtargets++] = *argv++;
			--argc;
		}
	}
	while (*envp != NULL) {
		if ((value = index(*envp, '=')) != NULL
		 && index(value, '$') == NULL) {
			s = *envp;
			while ((c=*s) != ' ' && c != '\t' && c != '=')
				++s;

			len = s - *envp;
			namesave=mmalloc(len+1);
			strncpy(namesave, *envp, len);
			namesave[len] = '\0';

			if (eflag)
				define(namesave, value+1, 1);
			else {
				starttoken();
				while (*++value) addtoken(*value);
				endtoken();
				define(namesave, token, 0);
			}
		}
		++envp;
	}
	srcpath = mexists("SRCPATH");
	suffixes=lookup(".SUFFIXES");
	if (!rflag)
		inlib(MACROFILE);
	deftarget = NULL;
	if (fp == NULL)
		inpath("makefile", "Makefile", NULL);
	else {
		fd = stdin;
		do {
			input( strcmp(fp->value, "-") == 0 ? NULL : fp->value);
			fp = fp->next;
		} while (fp != NULL);
	}
	if (!rflag)
	{
		inactionfile = 1;
		inlib(ACTIONFILE);
		inactionfile = 0;
	}

	if (sexists(".IGNORE") != NULL)
		++iflag;
	if (sexists(".SILENT") != NULL)
		++sflag;
	deflt = sexists(".DEFAULT");
	if (pflag){
		if (macro != NULL) {
			printf("Macros:\n");
			for (mp = macro; mp != NULL; mp=mp->next)
				printf("%s=%s\n", mp->name, mp->value);
		}
		printf("Rules:\n");
		for(sp=sym;sp!=NULL;sp=sp->next){
			if (sp->type!=T_UNKNOWN){
				printf("%s:", sp->name);
				if (sp->type==T_DETAIL)
					putchar(':');
				for(d=sp->deplist;d!=NULL;d=d->next)
					printf(" %s", d->symbol->name);
				printf("\n");
				if (sp->action)
					printf("\t%s\n", sp->action);
			}
		}
	}
	if (numtargets)

	{
		int i;

		for (i=0;i<numtargets;i++)
			make(lookup(dtarget[i]));		
	} else
		make(lookup(deftarget));

	exit(ALLOK);
}


/* end of make.c */
