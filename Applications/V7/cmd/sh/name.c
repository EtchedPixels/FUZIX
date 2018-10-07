/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

static BOOL chkid(const char *);
static void namwalk(NAMPTR);


NAMNOD ps2nod =  { (NAMPTR) NIL, (NAMPTR) NIL, ps2name  };
NAMNOD fngnod =  { (NAMPTR) NIL, (NAMPTR) NIL, fngname  };
NAMNOD pathnod = { (NAMPTR) NIL, (NAMPTR) NIL, pathname };
NAMNOD ifsnod =  { (NAMPTR) NIL, (NAMPTR) NIL, ifsname  };
NAMNOD ps1nod =  { &pathnod, &ps2nod, ps1name  };
NAMNOD homenod = { &fngnod,  &ifsnod, homename };
NAMNOD mailnod = { &homenod, &ps1nod, mailname };

NAMPTR namep = &mailnod;


/* ========	variable and string handling	======== */

int syslook(char *w, SYSTAB syswds)
{
	register char first;
	register const char *s;
	register SYSPTR syscan;

	syscan = syswds;
	first = *w;

	while ( (s = syscan->sysnam) ) {
		if (first == *s && eq(w, s))
			return (syscan->sysval);
		syscan++;
	}
	return (0);
}

void setlist(register ARGPTR arg, int xp)
{
	while (arg) {
		register char *s = mactrim(arg->argval);
		setname(s, xp);
		arg = arg->argnxt;
		if (flags & execpr) {
			prs(s);
			if (arg)
				blank();
			else
				newline();;
		}
	}
}

void setname(char *argi, int xp)
{
	register char *argscan = argi;
	register NAMPTR n;

	if (letter(*argscan)) {
		while (alphanum(*argscan))
			argscan++;
		if (*argscan == '=') {
			*argscan = 0;
			n = lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			if (xp & N_ENVNAM)
				n->namenv = n->namval = argscan;
			else
				assign(n, argscan);
			return;
		}
	}
	failed(argi, notid);
}

void replace(char **a, const char *v)
{
	sh_free(*a);
	*a = make(v);
}

void dfault(NAMPTR n, const char *v)
{
	if (n->namval == 0) {
		assign(n, v);
	}
}

void assign(NAMPTR n, const char *v)
{
	if (n->namflg & N_RDONLY)
		failed(n->namid, wtfailed);
	else
		replace((char **)&n->namval, v);	/* FIXME: check safe */
}

int readvar(char **names)
{
	static6502 FILEBLK fb;
	register FILE f = &fb;
	register char c;
	register int rc = 0;
	NAMPTR n = lookup(*names++);	/* done now to avoid storage mess */
	STKPTR rel = (STKPTR) relstak();

	push(f);
	initf(dup(0));

	if (lseek(0, 0L, 1) == -1)
		f->fsiz = 1;

	for (;;) {
		c = nextc(0);
		if ((*names && any(c, ifsnod.namval)) || eolchar(c)) {
			zerostak();
			assign(n, absstak(rel));
			setstak(rel);
			if (*names)
				n = lookup(*names++);
			else
				n = 0;
			if (eolchar(c))
				break;
		} else {
			pushstak(c);
		}
	}
	while (n) {
		assign(n, nullstr);
		if (*names)
			n = lookup(*names++);
		else
			n = 0;
	}
	if (eof)
		rc = 1;
	lseek(0, (long) (f->fnxt - f->fend), 1);
	pop();
	return rc;
}

void assnum(char **p, int i)
{
	itos(i);
	replace(p, numbuf);
}

char *make(const char *v)
{
	register char *p;

	if (v) {
		movstr(v, p = alloc(length(v)));
		return p;
	} else
		return 0;
}


NAMPTR lookup(register char *nam)
{
	register NAMPTR nscan = namep;
	register NAMPTR *prev;
	int LR;

	if (!chkid(nam))
		failed(nam, notid);

	while (nscan) {
		if ((LR = cf(nam, nscan->namid)) == 0)
			return (nscan);
		else if (LR < 0)
			prev = &(nscan->namlft);
		else
			prev = &(nscan->namrgt);

		nscan = *prev;
	}

	/* add name node */
	nscan = (NAMPTR) alloc(sizeof *nscan);
	nscan->namlft = nscan->namrgt = (NAMPTR) NIL;
	nscan->namid = make(nam);
	nscan->namval = 0;
	nscan->namflg = N_DEFAULT;
	nscan->namenv = 0;
	return (*prev = nscan);
}

static BOOL chkid(const char *nam)
{
	register const char *cp = nam;

	if (!letter(*cp))
		return (FALSE);
	else {
		while (*++cp) {
			if (!alphanum(*cp))
				return (FALSE);
		}
	}
	return TRUE;
}

static void (*namfn) (NAMPTR);

void namscan(void (*fn) (NAMPTR))
{
	namfn = fn;
	namwalk(namep);
}

static void namwalk(NAMPTR np)
{
	if (np) {
		namwalk(np->namlft);
		(*namfn) (np);
		namwalk(np->namrgt);
	}
}

void printnam(NAMPTR n)
{
	register const char *s;

	sigchk();
	if ( (s = n->namval) ) {
		prs(n->namid);
		prc('=');
		prs(s);
		newline();
	}
}

static char *staknam(register NAMPTR n)
{
	register char *p;

	p = movstr(n->namid, staktop);
	p = movstr("=", p);
	p = movstr(n->namval, p);
	return (getstak(p + 1 - ADR(stakbot)));
}

void exname(register NAMPTR n)
{
	if (n->namflg & N_EXPORT) {
		sh_free((void *)n->namenv);
		n->namenv = make(n->namval);
	} else {
		sh_free((void *)n->namval);
		n->namval = make(n->namenv);
	}
}

void printflg(register NAMPTR n)
{
	if (n->namflg & N_EXPORT) {
		prs(export);
		blank();
	}
	if (n->namflg & N_RDONLY) {
		prs(readonly);
		blank();
	}
	if (n->namflg & (N_EXPORT | N_RDONLY)) {
		prs(n->namid);
		newline();
	}
}

void sh_getenv(void)
{
	register STRING *e = environ;

	while (*e) {
		setname(*e++, N_ENVNAM);
	}
}

static int namec;

void countnam(NAMPTR n)
{
	namec++;
}

static char **argnam;

void pushnam(NAMPTR n)
{
	if (n->namval)
		*argnam++ = staknam(n);
}

char **sh_setenv(void)
{
	register char **er;

	namec = 0;
	namscan(countnam);
	argnam = er =
	    (STRING *) getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return er;
}
