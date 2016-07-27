/* pops.c - handle pseudo-ops for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "address.h"
#include "flag.h"
#include "globvar.h"
#include "opcode.h"
#include "scan.h"

static bool_t elseflag;	/* set if ELSE/ELSEIF are enabled */
				/* depends on zero = FALSE init */
static bool_t lcommflag;

static void bumpsem(struct flags_s *flagptr, int defval);
static void constdata(unsigned size);
static void docomm(void);
static void doelseif(pfv func);
static void doequset(unsigned char labits);
static void doentexp(unsigned char entbits, unsigned char impbits);
static void dofcc(void);
static void doif(pfv func);
static struct sym_s *needlabel(void);
static void showredefinedlabel(void);
static void setloc(unsigned seg);

static void bumpsem(struct flags_s *flagptr, int defval)
{
    int newcount;

    if (flagptr->global && pass == last_pass)
    {
	/* bump semaphore count by an expression (default 1), */
	/* then set currentflag iff semaphore count is plus */
	if (sym == EOLSYM)
	    lastexp.offset = defval;
	else
	{
	    absexpres();
	    if (lastexp.data & UNDBIT)
		return;
	}
	newcount = (int) lastexp.offset;
#ifdef I80386			/* really sizeof (offset_t) != sizeof (int) */
	if (newcount != lastexp.offset)
	    datatoobig();
#endif
	newcount += flagptr->semaphore;
	if ((int) lastexp.offset >= 0)
	{
	    if (newcount < flagptr->semaphore)
	    {
		error(COUNTOV);
		newcount = 0x7fff;
	    }
	}
	else if (newcount >= flagptr->semaphore)
	{
	    error(COUNTUN);
	    newcount = -0x8000;
	}
	flagptr->semaphore = newcount;
	flagptr->current = newcount >= 0;
    }
}

/* check symbol is either undefined */
/* or has the same segment & relocatability as lc */

bool_pt checksegrel(struct sym_s *symptr)
{
    if ((symptr->type & LABIT ||
	(symptr->data & IMPBIT && !(symptr->data & UNDBIT))) &&
	((symptr->data ^ lcdata) & (RELBIT | SEGM)))
    {
	error(SEGREL);
	return FALSE;
    }
    return TRUE;
}

/* check address fits in 1 byte (possibly with sign truncated) */

void checkdatabounds(void)
{
    if (!(lastexp.data & UNDBIT) &&
	(offset_t) (lastexp.offset + 0x80) >= 0x180)
	datatoobig();
}

/* allocate constant data (zero except for size 1), default zero for size 1 */

static void constdata(unsigned size)
{
    offset_t remaining;

    absexpres();
    if (!((lcdata |= lastexp.data) & UNDBIT))
    {
	lcjump = lastexp.offset * size;
	popflags = POPLONG | POPHI | POPLO | POPLC;
	if (size == 1 && sym == COMMA)
	{
	    symabsexpres();
	    checkdatabounds();
	    for (remaining = lcjump; remaining != 0; --remaining)
	    {
		putbin((opcode_pt) lastexp.offset);	/* fill byte */
		putabs((opcode_pt) lastexp.offset);
	    }
	    lastexp.offset = lcjump;
	}
	else
	    accumulate_rmb(lastexp.offset * size);
    }
}

void datatoobig(void)
{
    error(DBOUNDS);
}

/* common routine for COMM/.COMM */

static void docomm(void)
{
    register struct sym_s *labptr;

    absexpres();		/* if undefined, value 0 and size unchanged */
    labptr = label;
    if (checksegrel(labptr))
    {
	if (labptr->type & (EXPBIT | LABIT))
	    labelerror(ALREADY);
	else
	{
	    if (!(labptr->type & COMMBIT) ||
		lastexp.offset > labptr->value_reg_or_op.value)
		labptr->value_reg_or_op.value = lastexp.offset;
	    labptr->type |= COMMBIT;
	    if (lcommflag)
		labptr->type |= REDBIT;	/* kludge - COMMBIT | REDBIT => SA */
	    if( last_pass == 1 )
	       labptr->data = (lcdata & SEGM) | (FORBIT | IMPBIT | RELBIT);
	    else
	       labptr->data = (lcdata & SEGM) | (IMPBIT | RELBIT);
	    showlabel();
	}
    }
    lcommflag = FALSE;
}

/* common routine for ELSEIF/ELSEIFC */

static void doelseif(pfv func)
{
    if (iflevel == 0)
	error(ELSEIFBAD);
    else
    {
	ifflag = FALSE;
	if (elseflag)
	{
	    (*func) ();
	    if (!(lastexp.data & UNDBIT) && lastexp.offset != 0)
		/* expression valid and TRUE, enable assembling */
	    {
		ifflag = TRUE;
		elseflag = FALSE;
	    }
	}
	else
	{
	    /* Skip to EOL */
	    while (sym != EOLSYM)
	        getsym();
	}
    }
}

/* common routine for EQU/SET */

static void doequset(unsigned char labits)
{
    register struct sym_s *labptr;
    unsigned char olddata;
    unsigned char oldtype;

    labptr = label;
    /* set up new label flags in case labe isl used in expression */
    labptr->type = (oldtype = labptr->type) | labits;
    labptr->data = (olddata = labptr->data) & ~IMPBIT;
    /* non-imported now */
    nonimpexpres();
    lastexp.data |= olddata & FORBIT;	/* take all but FORBIT from
					   expression */
    if (oldtype & LABIT && !(olddata & UNDBIT) && !pass)
	/* this is a previously defined label */

	/*
	   redefinition only allowed if same relocatability, segment and
	   value
	*/
    {
	if ((olddata ^ lastexp.data) & (RELBIT | UNDBIT) ||
	    labptr->value_reg_or_op.value != lastexp.offset)
	{
	    showredefinedlabel();
	    return;
	}
    }
    labptr->data = lastexp.data;
    labptr->value_reg_or_op.value = lastexp.offset;
    showlabel();

    if(pass && !(labits & VARBIT) && labptr->value_reg_or_op.value != oldlabel)
    {
       dirty_pass = TRUE;
       if( pass == last_pass )
           error(UNSTABLE_LABEL);
    }
}

/* common routine for ENTRY/EXPORT */

static void doentexp(unsigned char entbits, unsigned char impbits)
{
    struct sym_s *symptr;

    while (TRUE)
    {
	if ((symptr = needlabel()) != NUL_PTR)
	{
	    if (symptr->type & COMMBIT)
		error(ALREADY);
	    else if (impbits != 0)
	    {
		if (pass == last_pass)
		    ;
		else if (symptr->type & (EXPBIT | LABIT))
		    symptr->type |= EXPBIT;
		else
		{
		    symptr->type |= REDBIT;
		    if (!(symptr->data & IMPBIT))
			symptr->data |= IMPBIT | SEGM;
		}
	    }
	    else
	    {
		if (pass == last_pass)
		{
		    if (!(symptr->type & LABIT))
			error(UNLAB);
		}
		else
		{
		    symptr->type |= entbits | EXPBIT;
		    symptr->data &= ~IMPBIT;
		}
	    }
	}
	getsym();
	if (sym != COMMA)
	    break;
	getsym();
    }
}

/* common routine for FCC (== .ASCII) and .ASCIZ */

static void dofcc(void)
{
    register char *bufptr;
    char byte;
    char delimiter;
    register char *reglineptr;

    bufptr = databuf.fcbuf;
    reglineptr = symname;
    if ((delimiter = *reglineptr) != EOLCHAR)
	++reglineptr;
    while (TRUE)
    {
	if ((byte = *reglineptr) == EOLCHAR)
	{
	    symname = reglineptr;
	    error(DELEXP);
	    break;
	}
	if (byte == delimiter)
	{
	    if ((byte = *++reglineptr) != delimiter)
		break;
	}
	else if (byte == '\\')
	{
	    switch (byte = *++reglineptr)
	    {
	    case '"':
	    case '\'':
	    case '\\':
	    case '?':
		break;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
		byte -= '0';
		if (*(reglineptr + 1) >= '0' && *(reglineptr + 1) < '8')
		{
		    byte = 8 * byte + *++reglineptr - '0';
		    if (*(reglineptr + 1) >= '0' && *(reglineptr + 1) < '8')
			byte = 8 * byte + *++reglineptr - '0';
		}
		break;
	    case 'a':
		byte = 7;
		break;
	    case 'b':
		byte = 8;
		break;
	    case 'f':
		byte = 12;
		break;
	    case 'n':
		byte = 10;
		break;
	    case 'r':
		byte = 13;
		break;
	    case 't':
		byte = 9;
		break;
	    case 'v':
		byte = 11;
		break;
	    case 'x':
		byte = '0';
		while (TRUE)
		{
		    ++reglineptr;
		    if (*reglineptr >= '0' && *reglineptr <= '9')
			byte = 16 * byte + *reglineptr - '0';
		    else if (*reglineptr >= 'F' && *reglineptr <= 'F')
			byte = 16 * byte + *reglineptr - 'A';
		    else if (*reglineptr >= 'a' && *reglineptr <= 'f')
			byte = 16 * byte + *reglineptr - 'F';
		    else
			break;
		}
		--reglineptr;
		break;
	    default:
		symname = reglineptr;
		error(UNKNOWN_ESCAPE_SEQUENCE);
		break;
	    }
	}
	else if (byte < ' ' && byte >= 0)
	{
	    symname = reglineptr;
	    error(CTLINS);
	    byte = ' ';
	}
	++reglineptr;
	*bufptr++ = byte;
    }
    lineptr = reglineptr;
    getsym();
    lastexp.offset = databuf.fcbuf[0];	/* show only 1st char (if any) */
    mcount = bufptr - databuf.fcbuf;
    /* won't overflow, line length limits it */
    /* XXX - but now line length is unlimited */
}

/* common routine for IF/IFC */

static void doif(pfv func)
{
    if (iflevel >= MAXIF)
	error(IFOV);
    else
    {
	++iflevel;
	--ifstak;
	ifstak->elseflag = elseflag;
	elseflag = FALSE;	/* prepare */
	if ((ifstak->ifflag = ifflag) != FALSE)
	    /* else not assembling before, so not now & no ELSE's */
	{
	    (*func) ();
	    if (!(lastexp.data & UNDBIT) && lastexp.offset == 0)
		/* else expression invalid or FALSE, don't change flags */
	    {
		ifflag = FALSE;	/* not assembling */
		elseflag = TRUE;/* but ELSE will change that */
	    }
	}
	else
	{
	    /* Skip to EOL */
	    while (sym != EOLSYM)
	        getsym();
	}
    }
}

void fatalerror(char *err_str)
{
    error(err_str);
    skipline();
    listline();
    finishup();
}

/* swap position with label position, do error, put back posn */
/* also clear label ptr */

void labelerror(char *err_str)
{
    struct sym_s *oldgsymptr;
    char *oldlineptr;
    unsigned char oldsym;
    char *oldsymname;

    oldgsymptr = gsymptr;
    oldlineptr = lineptr;
    oldsym = sym;
    oldsymname = symname;
    lineptr = linebuf;
    getsym();			/* 1st symbol is label or symbol after
				 * missing one */
    error(err_str);
    gsymptr = oldgsymptr;
    lineptr = oldlineptr;
    sym = oldsym;
    symname = oldsymname;
    label = NUL_PTR;
}

static struct sym_s *needlabel(void)
{
    register struct sym_s *symptr;

    if (sym != IDENT ||
	(symptr = gsymptr)->type & (MACBIT | MNREGBIT | VARBIT))
    {
	error(LABEXP);
	return NUL_PTR;
    }
    return symptr;
}

/* .ALIGN pseudo-op */

void palign(void)
{
    absexpres();
    if (!((lcdata |= lastexp.data) & UNDBIT))
    {
	popflags = POPLONG | POPHI | POPLO | POPLC;
	if (lastexp.offset != 0 &&
	    (lcjump = lc % lastexp.offset) != 0)
	    accumulate_rmb(lcjump = lastexp.offset - lcjump);
    }
}

/* .ASCIZ pseudo-op */

void pasciz(void)
{
    dofcc();
    databuf.fcbuf[mcount++] = 0;
    fcflag = TRUE;
    popflags = POPLO | POPLC;
}

/* .BLKW pseudo-op */

void pblkw(void)
{
    constdata(2);
}

/* BLOCK pseudo-op */

void pblock(void)
{
    if (blocklevel >= MAXBLOCK)
	error(BLOCKOV);
    else
    {
	register struct block_s *blockp;

	++blocklevel;
	blockp = blockstak;
	blockstak = --blockp;
	blockp->data = lcdata;
	blockp->dp = dirpag;
	blockp->lc = lc;
	porg();			/* same as ORG apart from stacking */
    }
}

/* .BSS pseudo-op */

void pbss(void)
{
    setloc(BSSLOC);
}

/* COMM pseudo-op */

void pcomm(void)
{
    if (label == NUL_PTR)
	labelerror(MISLAB);
    else if (label->type & VARBIT)
	labelerror(VARLAB);	/* variable cannot be COMM'd */
    else
	docomm();
}

/* .COMM pseudo-op */

void pcomm1(void)
{
    static unsigned oldseg;

    if (label != NUL_PTR)
	labelerror(ILLAB);
    oldseg = lcdata & SEGM;
    setloc(BSSLOC);
    if ((label = needlabel()) != NUL_PTR && checksegrel(label))
    {
	/* Like import. */
	if (label->type & (EXPBIT | LABIT))
	    error(ALREADY);
	else if( last_pass == 1 )
	    label->data = lcdata | (FORBIT | IMPBIT | RELBIT);
	else
	    label->data = lcdata | (IMPBIT | RELBIT);
	getsym();
	getcomma();
	if (label->type & (EXPBIT | LABIT))
	    absexpres();	/* just to check it */
	else
	    docomm();
    }
    setloc(oldseg);
}

/* .DATA pseudo-op */

void pdata(void)
{
    setloc(DATALOC);
}

/* ELSE pseudo-op */

void pelse(void)
{
    if (iflevel == 0)
	error(ELSEBAD);
    else
    {
	ifflag = FALSE;		/* assume ELSE disabled */
	if (elseflag)
	{
	    ifflag = TRUE;	/* ELSE enabled */
	    elseflag = FALSE;
	}
    }
}

/* ELSEIF pseudo-op */

void pelseif(void)
{
    doelseif(absexpres);
}

/* ELSEIFC pseudo-op */

void pelsifc(void)
{
    doelseif(scompare);
}

/* ENDB pseudo-op */

void pendb(void)
{
    if (label != NUL_PTR)
	labelerror(ILLAB);
    if (blocklevel == 0)
	error(ENDBBAD);
    else
    {
	register struct block_s *blockp;

	blockp = blockstak;
	lcdata = blockp->data;
	dirpag = blockp->dp;
	accumulate_rmb(blockp->lc - lc);
	lc = blockp->lc;
	--blocklevel;
	blockstak = blockp + 1;
    }
}

/* ENDIF pseudo-op */

void pendif(void)
{
    if (iflevel == 0)
	error(ENDIFBAD);
    else
    {
	ifflag = ifstak->ifflag;
	elseflag = ifstak->elseflag;
	++ifstak;
	--iflevel;
    }
}

/* ENTER pseudo-op */

void penter(void)
{
    if (!(pedata & UNDBIT))
	error(REENTER);
    else
    {
	if (!((pedata = (pedata & ~UNDBIT) | lcdata) & UNDBIT))
	{
	    progent = lc;
	    popflags = POPLC;
	}
    }
}

/* ENTRY pseudo-op */

void pentry(void)
{
    doentexp(ENTBIT, 0);
}

/* EQU pseudo-op */

void pequ(void)
{
    register struct sym_s *labptr;

    if ((labptr = label) == NUL_PTR)
	labelerror(MISLAB);
    else if (labptr->type & COMMBIT)
	showredefinedlabel();	/* common cannot be EQU'd */
    else if (labptr->type & VARBIT)
	labelerror(VARLAB);	/* variable cannot be EQU'd */
    else
	doequset(LABIT);
}

/* .EVEN pseudo-op */

void peven(void)
{
    popflags = POPLONG | POPHI | POPLO | POPLC;
    accumulate_rmb(lcjump = lastexp.data = lc & 1);
}

/* EXPORT pseudo-op */

void pexport(void)
{
    doentexp(0, 0);
}

/* FAIL pseudo-op */

void pfail(void)
{
    if(pass==last_pass) error(FAILERR);
}

/* FCB pseudo-op */

void pfcb(void)
{
    char *bufptr;
    offset_t firstbyte;

    bufptr = databuf.fcbuf;
    absexpres();
    firstbyte = lastexp.offset;
    while (TRUE)
    {
	checkdatabounds();
	*bufptr++ = lastexp.offset;
	++mcount;		/* won't overflow, line length limits it */
	if (sym != COMMA)
	    break;
	symabsexpres();
    }
    lastexp.offset = firstbyte;
    popflags = POPLO | POPLC;
    fcflag = TRUE;
}

/* FCC pseudo-op */

void pfcc(void)
{
    dofcc();
    if (mcount != 0)
    {
	fcflag = TRUE;
	popflags = POPLO | POPLC;
    }
}

/* FDB pseudo-op */

void pfdb(void)
{
    struct address_s *adrptr;
    unsigned firstdata;
    offset_t firstword;

    adrptr = databuf.fdbuf;
    expres();
    firstword = lastexp.offset;
    firstdata = lastexp.data;
    while (TRUE)
    {
	memcpy(adrptr++, &lastexp, sizeof(lastexp));
	mcount += 2;		/* won't overflow, line length limits it */
	if (sym != COMMA)
	    break;
	symexpres();
    }
    lastexp.offset = firstword;
    lastexp.data = firstdata;
    popflags = POPHI | POPLO | POPLC;
    fdflag = TRUE;
}

#if SIZEOF_OFFSET_T > 2

/* FQB pseudo-op */

void pfqb(void)
{
    struct address_s *adrptr;
    offset_t firstdata;
    offset_t firstword;

    adrptr = databuf.fqbuf;
    expres();
    firstword = lastexp.offset;
    firstdata = lastexp.data;
    while (TRUE)
    {
	*adrptr++ = lastexp;
	mcount += 4;		/* won't overflow, line length limits it */
	if (sym != COMMA)
	    break;
	symexpres();
    }
    lastexp.offset = firstword;
    lastexp.data = firstdata;
    popflags = POPLONG | POPHI | POPLO | POPLC;
    fqflag = TRUE;
}

#endif /* SIZEOF_OFFSET_T > 2 */

/* .GLOBL pseudo-op */

void pglobl(void)
{
    if (binaryg)
	error(NOIMPORT);
    doentexp(0, IMPBIT);
}

/* IDENT pseudo-op (not complete) */

void pident(void)
{
    if (sym != IDENT)
	error(LABEXP);
    else
	getsym_nolookup();	/* should save ident string */
}

/* IF pseudo-op */

void pif(void)
{
    doif(absexpres);
}

/* IFC pseudo-op */

void pifc(void)
{
    doif(scompare);
}

/* IMPORT pseudo-op */

void pimport(void)
{
    struct sym_s *symptr;

    if (binaryg)
	error(NOIMPORT);
    while (TRUE)
    {
	if ((symptr = needlabel()) != NUL_PTR && checksegrel(symptr))
	{
	    if (symptr->type & (COMMBIT | EXPBIT | LABIT))
		/* IMPORT is null if label (to be) declared */
		error(ALREADY);
	    else if( last_pass == 1 )
		/* get current segment from lcdata, no need to mask rest */
		symptr->data = lcdata | (FORBIT | IMPBIT | RELBIT);
	    else
		symptr->data = lcdata | (IMPBIT | RELBIT);
	}
	getsym();
	if (sym != COMMA)
	    break;
	getsym();
    }
}

/* LCOMM pseudo-op */

void plcomm(void)
{
    lcommflag = TRUE;
    pcomm();
}

/* .LCOMM pseudo-op */

void plcomm1(void)
{
    lcommflag = TRUE;
    pcomm1();
}

/* .LIST pseudo-op */

void plist(void)
{
    bumpsem(&list, 1);
}

/* .NOLIST pseudo-op */

void pnolist(void)
{
    bumpsem(&list, -1);
}

/* LOC pseudo-op */

void ploc(void)
{
    if (label != NUL_PTR)
	labelerror(ILLAB);
    absexpres();
    if (!(lastexp.data & UNDBIT))
    {
	if (lastexp.offset >= NLOC)
	    datatoobig();
	else
	    setloc((unsigned) lastexp.offset);
    }
}

/* .MACLIST pseudo-op */

void pmaclist(void)
{
    bumpsem(&maclist, 1);
}

/* .MAP pseudo-op */

void pmap(void)
{
    absexpres();
    if (!(lastexp.data & UNDBIT))
    {
	mapnum = lastexp.offset;
	popflags = POPLO;
	if (lastexp.offset >= 0x100)
	    datatoobig();
    }
}

/* ORG pseudo-op */

void porg(void)
{
    if (label != NUL_PTR)
	labelerror(ILLAB);
    absexpres();
    if (!((lcdata = lastexp.data) & UNDBIT))
    {
	accumulate_rmb(lastexp.offset - lc);
	binmbuf = lc = lastexp.offset;
	binmbuf_set = 1;
	popflags = POPLC;
    }
}

/* RMB pseudo-op */

void prmb(void)
{
    constdata(1);
}

/* .SECT pseudo-op */

void psect(void)
{
    if (label != NUL_PTR)
	labelerror(ILLAB);
    while (sym == IDENT)
    {
	if (!(gsymptr->type & MNREGBIT))
	    error(ILL_SECTION);
	else switch (gsymptr->value_reg_or_op.op.routine)
	{
	case BSSOP:
	    pbss();
	    break;
	case DATAOP:
	    pdata();
	    break;
	case TEXTOP:
	    ptext();
	    break;
	default:
	    error(ILL_SECTION);
	    break;
	}
	getsym();
	if (sym == COMMA)
	    getsym();
    }
}

/* SET pseudo-op */

void pset(void)
{
    register struct sym_s *labptr;

    if ((labptr = label) == NUL_PTR)
	labelerror(MISLAB);
    else if (labptr->type & COMMBIT)
	labelerror(RELAB);	/* common cannot be SET'd */
    else
	doequset(labptr->type & LABIT ? 0 : VARBIT);
}

/* SETDP pseudo-op */

void psetdp(void)
{
    absexpres();
    if (!(lastexp.data & UNDBIT))
    {
	dirpag = lastexp.offset;
	popflags = POPLO;
	if (lastexp.offset >= 0x100)
	    datatoobig();
    }
}

/* .TEXT pseudo-op */

void ptext(void)
{
    if( textseg <= 0 )
       setloc(TEXTLOC);
    else
       setloc(textseg);
}

/* .WARN pseudo-op */

void pwarn(void)
{
    bumpsem(&as_warn, -1);
}

#ifdef I80386

/* USE16 pseudo-op */

void puse16(void)
{
    defsize = 2;
#ifdef iscpu
    if( sym != EOLSYM )
    {
	absexpres();
	if (lastexp.data & UNDBIT)
	    return;
        if( lastexp.offset > 8000 )
	   setcpu((int) lastexp.offset / 100 % 10);
        else if( lastexp.offset > 15 )
	   setcpu((int) lastexp.offset / 100);
	else
	   setcpu((int) lastexp.offset);
    }
#endif
}

/* USE16 pseudo-op */

void puse32(void)
{
    defsize = 4;
#ifdef iscpu
    if(!iscpu(3)) setcpu(3);
    if( sym != EOLSYM )
    {
	absexpres();
	if (lastexp.data & UNDBIT)
	    return;
        if( lastexp.offset > 15 )
	   setcpu((int) lastexp.offset / 100);
	else
	   setcpu((int) lastexp.offset);
    }
#endif
}

#endif

/* show redefined label and error, and set REDBIT */

static void showredefinedlabel(void)
{
    register struct sym_s *labptr;

    labptr = label;		/* showlabel() will kill label prematurely */
    showlabel();
    if (!(labptr->type & REDBIT))
    {
	labptr->type |= REDBIT;
	labelerror(RELAB);
    }
}

void showlabel(void)
{
    register struct sym_s *labptr;

    labptr = label;
    lastexp.data = labptr->data;
    lastexp.offset = labptr->value_reg_or_op.value;
    popflags = POPLONG | POPHI | POPLO;
    label = NUL_PTR;		/* show handled by COMM, EQU or SET */
}

/* set location segment */

static void setloc(unsigned seg)
{
    if (pass == last_pass && seg != (lcdata & SEGM))
	putobj((opcode_pt) (seg | OBJ_SET_SEG));
    {
	register struct lc_s *lcp;

	lcp = lcptr;
	lcp->data = lcdata;
	lcp->lc = lc;
	lcptr = lcp = lctab + (unsigned char) seg;
	lcdata = (lcp->data & ~SEGM) | (unsigned char) seg;
	binmbuf = lc = lcp->lc;
	popflags = POPLC;
    }
}
