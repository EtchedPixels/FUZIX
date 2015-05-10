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

static IOPTR	inout();
static void	chkword();
static void	chksym();
static TREPTR	term();
static TREPTR	makelist();
static TREPTR	list();
static REGPTR	syncase();
static TREPTR	item();
static int	skipnl();
static void	prsym();
static void	synbad();


/* ========	command line decoding	========*/




TREPTR	makefork(flgs, i)
	INT		flgs;
	TREPTR		i;
{
	REG FORKPTR	t;

	t=(FORKPTR)getstak(FORKTYPE);
	t->forktyp=flgs|TFORK;
	t->forktre=i;
	t->forkio=0;
	return(TREPTR)(t);
}

static TREPTR	makelist(type,i,r)
	INT		type;
	TREPTR		i, r;
{
	REG LSTPTR	t;

	IF i==0 || r==0
	THEN	synbad();
	ELSE	t = (LSTPTR)getstak(LSTTYPE);
		t->lsttyp = type;
		t->lstlef = i; t->lstrit = r;
	FI
	return(TREPTR)(t);
}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

TREPTR	cmd(sym,flg)
	REG INT		sym;
	INT		flg;
{
	REG TREPTR	i, e;

	i = list(flg);

	IF wdval==NL
	THEN	IF flg&NLFLG
		THEN	wdval=';'; chkpr(NL);
		FI
	} else if ( i==0 && (flg&MTFLG)==0
	THEN	synbad();
	FI

	switch(wdval) {

	    case '&':
		IF i
		THEN	i = makefork(FINT|FPRS|FAMP, i);
		ELSE	synbad();
		FI

	    case ';':
		IF e=cmd(sym,flg|MTFLG)
		THEN	i=makelist(TLST, i, e);
		FI
		break;

	    case EOFSYM:
		IF sym==NL
		THEN	break;
		FI

	    default:
		IF sym
		THEN	chksym(sym);
		FI

	}
	return(i);
}

/*
 * list
 *	term
 *	list && term
 *	list || term
 */

static TREPTR	list(flg)
{
	REG TREPTR	r;
	REG INT		b;

	r = term(flg);
	WHILE r && ((b=(wdval==ANDFSYM)) || wdval==ORFSYM)
	DO	r = makelist((b ? TAND : TORF), r, term(NLFLG));
	OD
	return(r);
}

/*
 * term
 *	item
 *	item |^ term
 */

static TREPTR	term(flg)
{
	REG TREPTR	t;

	reserv++;
	IF flg&NLFLG
	THEN	skipnl();
	ELSE	word();
	FI

	IF (t=item(TRUE)) && (wdval=='^' || wdval=='|')
	THEN	return(makelist(TFIL, makefork(FPOU,t), makefork(FPIN|FPCL,term(NLFLG))));
	ELSE	return(t);
	FI
}

static REGPTR	syncase(esym)
	REG INT	esym;
{
	skipnl();
	IF wdval==esym
	THEN	return(0);
	ELSE	REG REGPTR	r=(REGPTR)getstak(REGTYPE);
		r->regptr=0;
		for(;;) {
		 wdarg->argnxt=r->regptr;
		     r->regptr=wdarg;
		     IF wdval || ( word()!=')' && wdval!='|' )
		     THEN synbad();
		     FI
		     IF wdval=='|'
		     THEN word();
		     ELSE break;
		     FI
		}
		r->regcom=cmd(0,NLFLG|MTFLG);
		IF wdval==ECSYM
		THEN	r->regnxt=syncase(esym);
		ELSE	chksym(esym);
			r->regnxt=0;
		FI
		return(r);
	FI
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

static TREPTR	item(flag)
	BOOL		flag;
{
	REG TREPTR	t;
	REG IOPTR	io;

	IF flag
	THEN	io=inout((IOPTR)0);
	ELSE	io=0;
	FI

	switch(wdval) {

	    case CASYM:
		{
		   t=(TREPTR)getstak(SWTYPE);
		   chkword();
		   ((SWPTR)t)->swarg=wdarg->argval;
		   skipnl(); chksym(INSYM|BRSYM);
		   ((SWPTR)t)->swlst=syncase(wdval==INSYM?ESSYM:KTSYM);
		   ((SWPTR)t)->swtyp=TSW;
		   break;
		}

	    case IFSYM:
		{
		   REG INT	w;
		   t=(TREPTR)getstak(IFTYPE);
		   ((IFPTR)t)->iftyp=TIF;
		   ((IFPTR)t)->iftre=cmd(THSYM,NLFLG);
		   ((IFPTR)t)->thtre=cmd(ELSYM|FISYM|EFSYM,NLFLG);
		   ((IFPTR)t)->eltre=((w=wdval)==ELSYM ? cmd(FISYM,NLFLG) : (w==EFSYM ? (wdval=IFSYM, item(0)) : 0));
		   IF w==EFSYM THEN return(t) FI
		   break;
		}

	    case FORSYM:
		{
		   t=(TREPTR)getstak(FORTYPE);
		   ((FORPTR)t)->fortyp=TFOR;
		   ((FORPTR)t)->forlst=0;
		   chkword();
		   ((FORPTR)t)->fornam=wdarg->argval;
		   IF skipnl()==INSYM
		   THEN	chkword();
			((FORPTR)t)->forlst=(COMPTR)item(0);
			IF wdval!=NL && wdval!=';'
			THEN	synbad();
			FI
			chkpr(wdval); skipnl();
		   FI
		   chksym(DOSYM|BRSYM);
		   ((FORPTR)t)->fortre=cmd(wdval==DOSYM?ODSYM:KTSYM,NLFLG);
		   break;
		}

	    case WHSYM:
	    case UNSYM:
		{
		   t=(TREPTR)getstak(WHTYPE);
		   ((WHPTR)t)->whtyp=(wdval==WHSYM ? TWH : TUN);
		   ((WHPTR)t)->whtre = cmd(DOSYM,NLFLG);
		   ((WHPTR)t)->dotre = cmd(ODSYM,NLFLG);
		   break;
		}

	    case BRSYM:
		t=cmd(KTSYM,NLFLG);
		break;

	    case '(':
		{
		   REG PARPTR	 p;
		   p=(PARPTR)getstak(PARTYPE);
		   p->partre=cmd(')',NLFLG);
		   p->partyp=TPAR;
		   t=makefork(0,p);
		   break;
		}

	    default:
		IF io==0
		THEN	return(0);
		FI

	    case 0:
		{
		   REG ARGPTR	argp;
		   REG ARGPTR	*argtail;
		   REG ARGPTR	*argset=0;
		   INT		keywd=1;
		   t=(TREPTR)getstak(COMTYPE);
		   ((COMPTR)t)->comio=io; /*initial io chain*/
		   argtail = &(((COMPTR)t)->comarg);
		   WHILE wdval==0
		   DO	argp = wdarg;
			IF wdset && keywd
			THEN	argp->argnxt=(ARGPTR)argset;
				argset=(ARGPTR *)argp;
			ELSE	*argtail=argp; argtail = &(argp->argnxt); keywd=flags&keyflg;
			FI
			word();
			IF flag
			THEN ((COMPTR)t)->comio=inout(((COMPTR)t)->comio);
			FI
		   OD

		   ((COMPTR)t)->comtyp=TCOM;
		   ((COMPTR)t)->comset=(ARGPTR)argset;
		   *argtail=0;
		   return(t);
		}

	}
	reserv++; word();
	IF io=inout(io)
	THEN	t=makefork(0,t); t->treio=io;
	FI
	return(t);
}


static int	skipnl()
{
	WHILE (reserv++, word()==NL) DO chkpr(NL) OD
	return(wdval);
}

static IOPTR	inout(lastio)
	IOPTR		lastio;
{
	REG INT		iof;
	REG IOPTR	iop;
	REG CHAR	c;

	iof=wdnum;

	switch(wdval) {

	    case DOCSYM:
		iof |= IODOC; break;

	    case APPSYM:
	    case '>':
		IF wdnum==0 THEN iof |= 1 FI
		iof |= IOPUT;
		IF wdval==APPSYM
		THEN	iof |= IOAPP; break;
		FI

	    case '<':
		IF (c=nextc(0))=='&'
		THEN	iof |= IOMOV;
		} else if ( c=='>'
		THEN	iof |= IORDW;
		ELSE	peekc=c|MARK;
		FI
		break;

	    default:
		return(lastio);
	}

	chkword();
	iop=(IOPTR)getstak(IOTYPE);
	iop->ioname=wdarg->argval;
	iop->iofile=iof;
	IF iof&IODOC
	THEN iop->iolst=iopend; iopend=iop;
	FI
	word(); iop->ionxt=inout(lastio);
	return(iop);
}

static void	chkword()
{
	IF word()
	THEN	synbad();
	FI
}

static void	chksym(sym)
{
	REG INT		x = sym&wdval;
	IF ((x&SYMFLG) ? x : sym) != wdval
	THEN	synbad();
	FI
}

static void	prsym(sym)
{
	IF sym&SYMFLG
	THEN	REG SYSPTR	sp=reserved;
		WHILE sp->sysval
			&& sp->sysval!=sym
		DO sp++ OD
		prs(sp->sysnam);
	} else if ( sym==EOFSYM
	THEN	prs(endoffile);
	ELSE	IF sym&SYMREP THEN prc(sym) FI
		IF sym==NL
		THEN	prs("newline");
		ELSE	prc(sym);
		FI
	FI
}

static void	synbad()
{
	prp(); prs(synmsg);
	IF (flags&ttyflg)==0
	THEN	prs(atline); prn(standin->flin);
	FI
	prs(colon);
	prc(LQ);
	IF wdval
	THEN	prsym(wdval);
	ELSE	prs(wdarg->argval);
	FI
	prc(RQ); prs(unexpected);
	newline();
	exitsh(SYNBAD);
}
