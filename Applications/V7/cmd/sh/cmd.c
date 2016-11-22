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
#include	"sym.h"

static IOPTR inout(IOPTR);
static void chkword(void);
static void chksym(int);
static TREPTR term(int);
static TREPTR makelist(int, TREPTR, TREPTR);
static TREPTR list(int);
static REGPTR syncase(int);
static TREPTR item(BOOL);
static int skipnl(void);
static void prsym(int);
static void synbad(void);


/* ========	command line decoding	========*/




TREPTR makefork(int flgs, TREPTR i)
{
	register FORKPTR t;

	t = (FORKPTR) getstak(FORKTYPE);
	t->forktyp = flgs | TFORK;
	t->forktre = i;
	t->forkio = 0;
	return (TREPTR) (t);
}

static TREPTR makelist(int type, TREPTR i, TREPTR r)
{
	register LSTPTR t;

	if (i == 0 || r == 0)
		synbad();
	else {
		t = (LSTPTR) getstak(LSTTYPE);
		t->lsttyp = type;
		t->lstlef = i;
		t->lstrit = r;
	}
	return (TREPTR) (t);
}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

TREPTR cmd(register int sym, int flg)
{
	register TREPTR i, e;

	i = list(flg);

	if (wdval == NL) {
		if (flg & NLFLG) {
			wdval = ';';
			chkpr(NL);
		}
	} else if (i == 0 && (flg & MTFLG) == 0) {
		synbad();
	}

	switch (wdval) {

	case '&':
		if (i)
			i = makefork(FINT | FPRS | FAMP, i);
		else
			synbad();

	case ';':
		if ( (e = cmd(sym, flg | MTFLG)) )
			i = makelist(TLST, i, e);
		break;

	case EOFSYM:
		if (sym == NL)
			break;

	default:
		if (sym)
			chksym(sym);
	}
	return i;
}

/*
 * list
 *	term
 *	list && term
 *	list || term
 */

static TREPTR list(int flg)
{
	register TREPTR r;
	register int b;

	r = term(flg);
	while (r && ((b = (wdval == ANDFSYM)) || wdval == ORFSYM)) {
		r = makelist((b ? TAND : TORF), r, term(NLFLG));
	}
	return r;
}

/*
 * term
 *	item
 *	item |^ term
 */

static TREPTR term(int flg)
{
	register TREPTR t;

	reserv++;
	if (flg & NLFLG)
		skipnl();
	else
		word();

	if ((t = item(TRUE)) && (wdval == '^' || wdval == '|')) {
		return (makelist(TFIL, makefork(FPOU, t),
			 makefork(FPIN | FPCL, term(NLFLG))));
	} else
		return (t);
}

static REGPTR syncase(register int esym)
{
	skipnl();
	if (wdval == esym) {
		return (0);
	} else {
		register REGPTR r = (REGPTR) getstak(REGTYPE);
		r->regptr = 0;
		for (;;) {
			wdarg->argnxt = r->regptr;
			r->regptr = wdarg;
			if (wdval || (word() != ')' && wdval != '|')) {
				synbad();
			}
			if (wdval == '|') {
				word();
			} else {
				break;
			}
		}
		r->regcom = cmd(0, NLFLG | MTFLG);
		if (wdval == ECSYM) {
			r->regnxt = syncase(esym);
		} else {
			chksym(esym);
			r->regnxt = 0;
		}
		return (r);
	}
}

/*
 * item
 *
 *	( cmd ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	if ... then ... else ... fi
 *	for ... while ... do ... done
 *	case ... in ... esac
 *	begin ... end
 */

static TREPTR item(BOOL flag)
{
	register TREPTR t;
	register IOPTR io;

	if (flag)
		io = inout((IOPTR) 0);
	else
		io = 0;

	switch (wdval) {
	case CASYM:
	{
		t = (TREPTR) getstak(SWTYPE);
		chkword();
		((SWPTR) t)->swarg = wdarg->argval;
		skipnl();
		chksym(INSYM | BRSYM);
		((SWPTR) t)->swlst = syncase(wdval == INSYM ? ESSYM : KTSYM);
		((SWPTR) t)->swtyp = TSW;
		break;
	}
	case IFSYM:
	{
		register int w;
		t = (TREPTR) getstak(IFTYPE);
		((IFPTR) t)->iftyp = TIF;
		((IFPTR) t)->iftre = cmd(THSYM, NLFLG);
		((IFPTR) t)->thtre = cmd(ELSYM | FISYM | EFSYM, NLFLG);
		((IFPTR) t)->eltre = ((w = wdval) == ELSYM ?
				cmd(FISYM, NLFLG) :
				(w == EFSYM ? (wdval = IFSYM, item(0)) : 0));
		if (w == EFSYM)
			return (t);
		break;
	}

	case FORSYM:
	{
		t = (TREPTR) getstak(FORTYPE);
		((FORPTR) t)->fortyp = TFOR;
		((FORPTR) t)->forlst = 0;
		chkword();
		((FORPTR) t)->fornam = wdarg->argval;
		if (skipnl() == INSYM) {
			chkword();
			((FORPTR) t)->forlst = (COMPTR) item(0);
			if (wdval != NL && wdval != ';')
				synbad();
			chkpr(wdval);
			skipnl();
		}
		chksym(DOSYM | BRSYM);
		((FORPTR) t)->fortre =
			    cmd(wdval == DOSYM ? ODSYM : KTSYM, NLFLG);
		break;
	}

	case WHSYM:
	case UNSYM:
	{
		t = (TREPTR) getstak(WHTYPE);
		((WHPTR) t)->whtyp = (wdval == WHSYM ? TWH : TUN);
		((WHPTR) t)->whtre = cmd(DOSYM, NLFLG);
		((WHPTR) t)->dotre = cmd(ODSYM, NLFLG);
		break;
	}

	case BRSYM:
		t = cmd(KTSYM, NLFLG);
		break;

	case '(':
	{
		register PARPTR p;
		p = (PARPTR) getstak(PARTYPE);
		p->partre = cmd(')', NLFLG);
		p->partyp = TPAR;
		t = makefork(0, /*FIXME*/(void *)p);
		break;
	}

	default:
		if (io == 0)
			return (0);

	case 0:
	{
		register ARGPTR argp;
		register ARGPTR *argtail;
		register ARGPTR *argset = 0;
		int keywd = 1;
		t = (TREPTR) getstak(COMTYPE);
		((COMPTR) t)->comio = io;	/*initial io chain */
		argtail = &(((COMPTR) t)->comarg);
		while (wdval == 0) {
			argp = wdarg;
			if (wdset && keywd) {
				argp->argnxt = (ARGPTR) argset;
				argset = (ARGPTR *) argp;
			} else {
				*argtail = argp;
				argtail = &(argp->argnxt);
				keywd = flags & keyflg;
			}
			word();
			if (flag)
				((COMPTR) t)->comio = inout(((COMPTR) t)->comio);
		}

		((COMPTR) t)->comtyp = TCOM;
		((COMPTR) t)->comset = (ARGPTR) argset;
		*argtail = 0;
		return (t);
	}

	}
	reserv++;
	word();
	if ( (io = inout(io)) ) {
		t = makefork(0, t);
		t->treio = io;
	}
	return t;
}


static int skipnl(void)
{
	while (reserv++, word() == NL)
		chkpr(NL);
	return wdval;
}

static IOPTR inout(IOPTR lastio)
{
	register int iof;
	register IOPTR iop;
	register CHAR c;

	iof = wdnum;

	switch (wdval) {

	case DOCSYM:
		iof |= IODOC;
		break;

	case APPSYM:
	case '>':
		if (wdnum == 0)
			iof |= 1;
		iof |= IOPUT;
		if (wdval == APPSYM) {
			iof |= IOAPP;
			break;
		}

	case '<':
		if ((c = nextc(0)) == '&')
			iof |= IOMOV;
		else if (c == '>')
			iof |= IORDW;
		else
			peekc = c | MARK;
		break;

	default:
		return (lastio);
	}

	chkword();
	iop = (IOPTR) getstak(IOTYPE);
	iop->ioname = wdarg->argval;
	iop->iofile = iof;
	if (iof & IODOC) {
		iop->iolst = iopend;
		iopend = iop;
	}
	word();
	iop->ionxt = inout(lastio);
	return (iop);
}

static void chkword(void)
{
	if (word())
		synbad();
}

static void chksym(int sym)
{
	register int x = sym & wdval;
	if (((x & SYMFLG) ? x : sym) != wdval)
		synbad();
}

static void prsym(int sym)
{
	if (sym & SYMFLG) {
		register SYSPTR sp = reserved;
		while (sp->sysval && sp->sysval != sym)
			sp++;
		prs(sp->sysnam);
	} else if (sym == EOFSYM) {
		prs(endoffile);
	} else {
		if (sym & SYMREP)
			prc(sym);
		if (sym == NL)
			prs("newline");
		else
			prc(sym);
	}
}

static void synbad(void)
{
	prp();
	prs(synmsg);
	if ((flags & ttyflg) == 0) {
		prs(atline);
		prn(standin->flin);
	}
	prs(colon);
	prc(LQ);
	if (wdval)
		prsym(wdval);
	else
		prs(wdarg->argval);
	prc(RQ);
	prs(unexpected);
	newline();
	exitsh(SYNBAD);
}
