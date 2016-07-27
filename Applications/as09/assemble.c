/* assemble.c - main loop for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "address.h"
#include "globvar.h"
#include "opcode.h"
#include "scan.h"

static bool_t nocolonlabel;	/* set for labels not followed by ':' */
static void (*routine)(void);
#ifdef I80386
static opcode_t rep = 0;       /* which rep/repne prefix was seen */
#endif
static pfv rout_table[] =
{
    pelse,
    pelseif,
    pelsifc,
    pendif,
    pif,
    pifc,

    /* start of non-conditionals */
    palign,
    pasciz,
    pblkw,
    pblock,
    pbss,
    pcomm,
    pcomm1,
    pdata,
    pendb,
    penter,
    pentry,
    pequ,
    peven,
    pexport,
    pfail,
    pfcb,
    pfcc,
    pfdb,
#if SIZEOF_OFFSET_T > 2
    pfqb,
#endif
    pget,
    pglobl,
    pident,
    pimport,
    plcomm,
    plcomm1,
    plist,
    ploc,
    pmaclist,
    pmacro,
    pmap,
    porg,
    pproceof,
    prmb,
    psect,
    pset,
    psetdp,
    ptext,
#ifdef I80386
    puse16,
    puse32,
#endif
    pwarn,
    /* end of pseudo-ops */

#ifdef I80386
    mbcc,
    mbswap,
    mcall,
    mcalli,
    mdivmul,
    menter,
    mEwGw,
    mExGx,
    mf_inher,
    mf_m,
    mf_m2,
    mf_m2_ax,
    mf_m2_m4,
    mf_m2_m4_m8,
    mf_m4_m8_optst,
    mf_m4_m8_st,
    mf_m4_m8_stst,
    mf_m4_m8_m10_st,
    mf_m10,
    mf_optst,
    mf_st,
    mf_stst,
    mf_w_inher,
    mf_w_m,
    mf_w_m2,
    mf_w_m2_ax,
    mgroup1,
    mgroup2,
    mgroup6,
    mgroup7,
    mgroup8,
    mGvEv,
    mGvMa,
    mGvMp,
    mimul,
    min,
    mincdec,
    minher,
    minher16,
    minher32,
    minhera,
    mint,
    mjcc,
    mjcxz,
    mlea,
    mmov,
    mmovx,
    mnegnot,
    mout,
    mpushpop,
    mret,
    mseg,
    msetcc,
    mshdouble,
    mtest,
    mxchg,
#endif /* I80386 */

#ifdef MC6809
    mall,			/* all address modes allowed, like LDA */
    malter,			/* all but immediate, like STA */
    mimmed,			/* immediate only (ANDCC, ORCC) */
    mindex,			/* indexed (LEA's) */
    minher,			/* inherent, like CLC or CLRA */
    mlong,			/* long branches */
    mshort,			/* short branches */
    msstak,			/* S-stack	(PSHS, PULS) */
    mswap,			/* TFR, EXG */
    mustak,			/* U-stack	(PSHU,PULU) */
#endif /* MC6809 */
};

static void asline(void);

/*
  This uses registers as follows: A is for work and is not preserved by
  the subroutines.B holds the last symbol code, X usually points to data
  about the last symbol, U usually holds the value of last expression
  or symbol, and Y points to the current char. The value in Y is needed
  by READCH and GETSYM.  EXPRES needs B and Y, and returns a value in U.
  If the expression starts with an identifier, X must point to its string.
  LOOKUP needs a string pointer in X and length in A. It returns a table
  pointer in X (unless not assembling and not found), symbol type in A
  and overflow in CC.
*/

void assemble(void)
{
    while (TRUE)
    {
	asline();
	if (label != NUL_PTR)	/* must be confirmed if still set */
	{			/* it is nulled by EQU,	COMM and SET */
#ifndef MC6809
#define NEEDENDLABEL ILLAB
	    if (nocolonlabel)
		error(NEEDENDLABEL);
#endif
	    if(pass && label->value_reg_or_op.value != oldlabel)
	    {
	       dirty_pass = TRUE;
	       if( pass == last_pass )
	          error(UNSTABLE_LABEL);
            }

	    label->type |= LABIT;	/* confirm, perhaps redundant */
	    if (label->type & REDBIT)
	    {
		/* REDBIT meant 'GLOBLBIT' while LABIT was not set. */
		label->type |= EXPBIT;
		label->type &= ~REDBIT;
	    }
	    if ((mcount | popflags) == 0)
		/* unaccompanied label, display adr like EQU and SET */
		showlabel();
	    label = NUL_PTR;	/* reset for next line */
	}
        skipline();
	listline();
	genbin();
	genobj();
	binmbuf = lc += lcjump
#ifdef I80386
	    + immcount
#endif
	    ;
    }
}

static void asline(void)
{
    register struct sym_s *symptr;

    postb = popflags = pcrflag =
#ifdef I80386
	sprefix = oprefix = aprefix =
#endif
	immcount = lastexp.data = lcjump = 0;
#ifdef I80386
    sib = NO_SIB;
#endif
#if SIZEOF_OFFSET_T > 2
    fqflag =
#endif
	fdflag = fcflag = FALSE;
    cpuwarn();
    readline();
    getsym();
    if (sym != IDENT)		/* expect label, mnemonic or macro */
    {	
       /* Warn if not a comment marker or a hash (for /lib/cpp) */
       if( sym != EOLSYM && sym != IMMEDIATE )
          list_force = TRUE;
       return;			/* anything else is a comment */
    }
    symptr = gsymptr;
    if (!ifflag)
	/* not assembling, just test for IF/ELSE/ELSEIF/ENDIF */
    {
	if (symptr == NUL_PTR || !(symptr->type & MNREGBIT) ||
	    symptr->data & REGBIT ||
	    symptr->value_reg_or_op.op.routine >= MIN_NONCOND)
	    return;
    }
    else if (!(symptr->type & (MACBIT | MNREGBIT)))
	/* not macro, op, pseudo-op or register, expect label */
    {
	oldlabel = symptr->value_reg_or_op.value;

	if ((nocolonlabel = (*lineptr - ':')) == 0)	/* exported label? */
	{
	    sym = COLON;
	    ++lineptr;
	}
	if (symptr->type & (LABIT | VARBIT))
	{
	    if (symptr->type & REDBIT)
		labelerror(RELAB);
	    label = symptr;

	    if (pass && !(symptr->type & VARBIT) /*&& last_pass>1*/)
	    {
	       label->data = (label->data & FORBIT) | lcdata;
	       label->value_reg_or_op.value = lc;
	    }
	}
	else if (checksegrel(symptr))
	{
	    symptr->type &= ~COMMBIT;	/* ignore COMM, PCOMM gives warning */
#ifdef MC6809
#if 0
	    if (sym == COLON)
		symptr->type |= EXPBIT;
#endif
#endif
				/* remember if forward referenced */
	    symptr->data = (symptr->data & FORBIT) | lcdata;
	    symptr->value_reg_or_op.value = lc;
				/* unless changed by EQU,COMM or SET */
	    label = symptr;
	}

	getsym();
	if (sym != IDENT)
	{
	    if (sym == EQOP)
	    {
		getsym();
		pequ();
	    }
	    return;		/* anything but ident is comment */
	}
	symptr = gsymptr;
    }
    if (symptr->type & MACBIT)
    {
	entermac(symptr);
	return;
    }
    if (!(symptr->type & MNREGBIT))
    {
	error(OPEXP);
	return;
    }
    if (symptr->data & REGBIT)
    {
	error(REGUID);
	return;
    }
    mnsize = 0;
    if ((page = (symptr->data & (PAGE1 | PAGE2))) != 0)
    {
#ifdef MNSIZE
	if (page == (PAGE1 | PAGE2))
	{
	    mnsize = 1;
	    page = 0;
	}
	else
#endif
	{
#ifdef PAGE2_OPCODE
	    if (page == PAGE2)
		page = PAGE2_OPCODE;
	    else
#endif
		page = PAGE1_OPCODE;
	    mcount = 1;
	}
    }
    opcode = symptr->value_reg_or_op.op.opcode;
#ifdef I80386
    needcpu((page==0 && ((opcode&0xF0) == 0x60||(opcode&0xF6)==0xC0))?1:0);
#endif
    routine = rout_table[symptr->value_reg_or_op.op.routine];
    getsym();
    (*routine)();
#ifdef I80386
    /* We handle "rep[ne]" refix as separate instruction; check if its use is valid */
    if (opcode == 0xF2 || opcode == 0xF3) {    /* REP */
        rep = opcode;
    /* Not another prefix */
    } else if (opcode != 0x2E &&    /* CSEG */
        opcode != 0x3E &&           /* DSEG */
        opcode != 0x26 &&           /* ESEG */
        opcode != 0x64 &&           /* FSEG */
        opcode != 0x65 &&           /* GSEG */
        opcode != 0x36 &&           /* SSEG */
        opcode != 0xF0) {           /* LOCK */
        if (rep == 0xF2 && (opcode&0xF6) != 0xA6)       /* REPNE CMPS/SCAS */
            error (REPNE_STRING);
        if (rep == 0xF3 && !((opcode&0xFC) == 0x6C ||   /* REP INS/OUTS */
            (opcode&0xFC) == 0xA4 ||                    /* REP MOVS/CMPS */
            (opcode&0xFC) == 0xAC ||                    /* REP SCAS/LODS */
            (opcode&0xFE) == 0xAA))                     /* REP STOS */
            error (REP_STRING);
        rep = 0;
    }
#endif
    if (sym != EOLSYM)
	error(JUNK_AFTER_OPERANDS);
#ifdef I80386
    needcpu(page==PAGE1_OPCODE?2:0);

    if (aprefix != 0)
	++mcount;
    if (oprefix != 0)
	++mcount;
    if (sprefix != 0)
	++mcount;
#endif
}
