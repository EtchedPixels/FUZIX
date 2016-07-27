/* mops.c - handle pseudo-ops */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "globvar.h"
#include "opcode.h"
#include "scan.h"
#undef EXTERN
#define EXTERN
#include "address.h"

#define is8bitadr(offset) ((offset_t) offset < 0x100)
#define is8bitsignedoffset(offset) ((offset_t) (offset) + 0x80 < 0x100)
#define pass2 (pass==last_pass)

static void mshort2(void);
static reg_pt regchk(void);
static void reldata(void);
static void segadj(void);

#ifdef I80386

#define iswordadr(offset) ((offset_t) (offset) < 0x10000L)
#define iswordoffset(offset) ((offset_t) (offset) + 0x8000L < 0x10000L)
#define iswordorswordoffset(offset) ((offset_t) (offset) + 0xFFFFL < 0x1FFFEL)

#define BYTE_SEGWORD        0x00
#define isspecreg(r) ((r) >= CR0REG && (r) <= TR7REG)

#define BASE_MASK   0x07
#define BASE_SHIFT     0
#define INDEX_MASK  0x38
#define INDEX_SHIFT    3
#define MOD_MASK    0xC0
# define REG_MOD    0xC0
# define MEM0_MOD   0x00
# define MEM1_MOD   0x40
# define MEM2_MOD   0x80
#define REG_MASK    0x38
#define REG_SHIFT      3
#define RM_MASK     0x07
#define RM_SHIFT       0
# define D16_RM     0x06
# define D32_RM     0x05
# define SIB_NOBASE 0x05
# define SIB_RM     0x04
#define SREG_MASK   0x38
#define SREG_SHIFT     3
#define SS_MASK     0xC0
#define SS_SHIFT       6

#define SEGMOV      0x04
#define SIGNBIT     0x02
#define TOREGBIT    0x02
#define WORDBIT     0x01

static opcode_t baseind16[] =
{
    0x00,			/* BP + BP, illegal */
    0x00,			/* BX + BP, illegal */
    0x03,			/* DI + BP */
    0x02,			/* SI + BP */
    0x00,			/* BP + BX, illegal */
    0x00,			/* BX + BX, illegal */
    0x01,			/* DI + BX */
    0x00,			/* SI + BX */
    0x03,			/* BP + DI */
    0x01,			/* BX + DI */
    0x00,			/* DI + DI, illegal */
    0x00,			/* SI + DI, illegal */
    0x02,			/* BP + SI */
    0x00,			/* BX + SI */
    0x00,			/* DI + SI, illegal */
    0x00,			/* SI + SI, illegal */
};

static opcode_t regbits[] =
{
    0x05 << REG_SHIFT,		/* BP */
    0x03 << REG_SHIFT,		/* BX */
    0x07 << REG_SHIFT,		/* DI */
    0x06 << REG_SHIFT,		/* SI */

    0x00 << REG_SHIFT,		/* EAX */
    0x05 << REG_SHIFT,		/* EBP */
    0x03 << REG_SHIFT,		/* EBX */
    0x01 << REG_SHIFT,		/* ECX */
    0x07 << REG_SHIFT,		/* EDI */
    0x02 << REG_SHIFT,		/* EDX */
    0x06 << REG_SHIFT,		/* ESI */
    0x04 << REG_SHIFT,		/* ESP */

    0x00 << REG_SHIFT,		/* AX */
    0x01 << REG_SHIFT,		/* CX */
    0x02 << REG_SHIFT,		/* DX */
    0x04 << REG_SHIFT,		/* SP */

    0x04 << REG_SHIFT,		/* AH */
    0x00 << REG_SHIFT,		/* AL */
    0x07 << REG_SHIFT,		/* BH */
    0x03 << REG_SHIFT,		/* BL */
    0x05 << REG_SHIFT,		/* CH */
    0x01 << REG_SHIFT,		/* CL */
    0x06 << REG_SHIFT,		/* DH */
    0x02 << REG_SHIFT,		/* DL */

    0x01 << REG_SHIFT,		/* CS */
    0x03 << REG_SHIFT,		/* DS */
    0x00 << REG_SHIFT,		/* ES */
    0x04 << REG_SHIFT,		/* FS */
    0x05 << REG_SHIFT,		/* GS */
    0x02 << REG_SHIFT,		/* SS */

    0x00 << REG_SHIFT,		/* CR0 */
    0x02 << REG_SHIFT,		/* CR2 */
    0x03 << REG_SHIFT,		/* CR3 */

    0x00 << REG_SHIFT,		/* DR0 */
    0x01 << REG_SHIFT,		/* DR1 */
    0x02 << REG_SHIFT,		/* DR2 */
    0x03 << REG_SHIFT,		/* DR3 */
    0x06 << REG_SHIFT,		/* DR6 */
    0x07 << REG_SHIFT,		/* DR7 */

    0x03 << REG_SHIFT,		/* TR3 */
    0x04 << REG_SHIFT,		/* TR4 */
    0x05 << REG_SHIFT,		/* TR5 */
    0x06 << REG_SHIFT,		/* TR6 */
    0x07 << REG_SHIFT,		/* TR7 */

    0x00 << REG_SHIFT,		/* ST(0) */
    0x01 << REG_SHIFT,		/* ST(1) */
    0x02 << REG_SHIFT,		/* ST(2) */
    0x03 << REG_SHIFT,		/* ST(3) */
    0x04 << REG_SHIFT,		/* ST(4) */
    0x05 << REG_SHIFT,		/* ST(5) */
    0x06 << REG_SHIFT,		/* ST(6) */
    0x07 << REG_SHIFT,		/* ST(7) */
};

static opsize_t regsize[] =
{
    2,				/* BP */
    2,				/* BX */
    2,				/* DI */
    2,				/* SI */

    4,				/* EAX */
    4,				/* EBP */
    4,				/* EBX */
    4,				/* ECX */
    4,				/* EDI */
    4,				/* EDX */
    4,				/* ESI */
    4,				/* ESP */

    2,				/* AX */
    2,				/* CX */
    2,				/* DX */
    2,				/* SP */

    1,				/* AH */
    1,				/* AL */
    1,				/* BH */
    1,				/* BL */
    1,				/* CH */
    1,				/* CL */
    1,				/* DH */
    1,				/* DL */

    2,				/* CS */
    2,				/* DS */
    2,				/* ES */
    2,				/* FS */
    2,				/* GS */
    2,				/* SS */

    4,				/* CR0 */
    4,				/* CR2 */
    4,				/* CR3 */

    4,				/* DR0 */
    4,				/* DR1 */
    4,				/* DR2 */
    4,				/* DR3 */
    4,				/* DR6 */
    4,				/* DR7 */

    4,				/* TR3 */
    4,				/* TR4 */
    4,				/* TR5 */
    4,				/* TR6 */
    4,				/* TR7 */

    10,				/* ST(0) */
    10,				/* ST(1) */
    10,				/* ST(2) */
    10,				/* ST(3) */
    10,				/* ST(4) */
    10,				/* ST(5) */
    10,				/* ST(6) */
    10,				/* ST(7) */

    0,				/* NOREG */
};

static opcode_t regsegword[] =
{
    WORDBIT,			/* BP */
    WORDBIT,			/* BX */
    WORDBIT,			/* DI */
    WORDBIT,			/* SI */

    WORDBIT,			/* EAX */
    WORDBIT,			/* EBP */
    WORDBIT,			/* EBX */
    WORDBIT,			/* ECX */
    WORDBIT,			/* EDI */
    WORDBIT,			/* EDX */
    WORDBIT,			/* ESI */
    WORDBIT,			/* ESP */

    WORDBIT,			/* AX */
    WORDBIT,			/* CX */
    WORDBIT,			/* DX */
    WORDBIT,			/* SP */

    BYTE_SEGWORD,		/* AH */
    BYTE_SEGWORD,		/* AL */
    BYTE_SEGWORD,		/* BH */
    BYTE_SEGWORD,		/* BL */
    BYTE_SEGWORD,		/* CH */
    BYTE_SEGWORD,		/* CL */
    BYTE_SEGWORD,		/* DH */
    BYTE_SEGWORD,		/* DL */

    SEGMOV,			/* CS */
    SEGMOV,			/* DS */
    SEGMOV,			/* ES */
    SEGMOV,			/* FS */
    SEGMOV,			/* GS */
    SEGMOV,			/* SS */

    0x20,			/* CR0 */
    0x20,			/* CR2 */
    0x20,			/* CR3 */

    0x21,			/* DR0 */
    0x21,			/* DR1 */
    0x21,			/* DR2 */
    0x21,			/* DR3 */
    0x21,			/* DR6 */
    0x21,			/* DR7 */

    0x24,			/* TR3 */
    0x24,			/* TR4 */
    0x24,			/* TR5 */
    0x24,			/* TR6 */
    0x24,			/* TR7 */

    0x00,			/* ST(0) */
    0x00,			/* ST(1) */
    0x00,			/* ST(2) */
    0x00,			/* ST(3) */
    0x00,			/* ST(4) */
    0x00,			/* ST(5) */
    0x00,			/* ST(6) */
    0x00,			/* ST(7) */

    0x00,			/* NOREG */
};

static opcode_t rm[] =
{
    0x05,			/* BP */
    0x03,			/* BX */
    0x07,			/* DI */
    0x06,			/* SI */

    0x00,			/* EAX */
    0x05,			/* EBP */
    0x03,			/* EBX */
    0x01,			/* ECX */
    0x07,			/* EDI */
    0x02,			/* EDX */
    0x06,			/* ESI */
    0x04,			/* ESP */

    0x00,			/* AX */
    0x01,			/* CX */
    0x02,			/* DX */
    0x04,			/* SP */

    0x04,			/* AH */
    0x00,			/* AL */
    0x07,			/* BH */
    0x03,			/* BL */
    0x05,			/* CH */
    0x01,			/* CL */
    0x06,			/* DH */
    0x02,			/* DL */

    0x01,			/* CS */
    0x03,			/* DS */
    0x00,			/* ES */
    0x04,			/* FS */
    0x05,			/* GS */
    0x02,			/* SS */

    0x00,			/* CR0 */
    0x00,			/* CR2 */
    0x00,			/* CR3 */

    0x00,			/* DR0 */
    0x00,			/* DR1 */
    0x00,			/* DR2 */
    0x00,			/* DR3 */
    0x00,			/* DR6 */
    0x00,			/* DR7 */

    0x00,			/* TR3 */
    0x00,			/* TR4 */
    0x00,			/* TR5 */
    0x00,			/* TR6 */
    0x00,			/* TR7 */

    0x00,			/* ST(0) */
    0x00,			/* ST(1) */
    0x00,			/* ST(2) */
    0x00,			/* ST(3) */
    0x00,			/* ST(4) */
    0x00,			/* ST(5) */
    0x00,			/* ST(6) */
    0x00,			/* ST(7) */

    0x04,			/* null index reg for sib only */
};

static opcode_t rmfunny[] =
{
    0x06,			/* BP */
    0x07,			/* BX */
    0x05,			/* DI */
    0x04,			/* SI */
};

static opcode_t segoverride[] =
{
    0x2E,			/* CS */
    0x3E,			/* DS */
    0x26,			/* ES */
    0x64,			/* FS */
    0x65,			/* GS */
    0x36,			/* SS */
};

static opcode_t ss[] =		/* scale to ss bits */
{
    0x00,			/* x0, illegal */
    0x00 << SS_SHIFT,		/* x1 */
    0x01 << SS_SHIFT,		/* x2 */
    0x00,			/* x3, illegal */
    0x02 << SS_SHIFT,		/* x4 */
    0x00,			/* x5, illegal */
    0x00,			/* x6, illegal */
    0x00,			/* x7, illegal */
    0x03 << SS_SHIFT,		/* x8 */
};

static unsigned char calljmp_kludge;
static opcode_t direction;
static bool_t fpreg_allowed;
static opcode_t segword;
/*
  Values of segword:
    BYTE_SEGWORD for byte ea's.
    SEGMOV for segment registers
    opcode for special registers
    WORDBIT for other word and dword ea's
*/

static struct ea_s source;
static struct ea_s source2;
static struct ea_s target;

static void Eb(struct ea_s *eap);
static void Ew(struct ea_s *eap);
static void Ev(struct ea_s *eap);
static void Ex(struct ea_s *eap);
static void Gd(struct ea_s *eap);
static void Gw(struct ea_s *eap);
static void Gv(struct ea_s *eap);
static void Gx(struct ea_s *eap);
static void buildea(struct ea_s *eap);
static void buildfloat(void);
static void buildfreg(void);
static void buildimm(struct ea_s *eap, bool_pt signflag);
static void buildregular(void);
static void buildsegword(struct ea_s *eap);
static void buildunary(opcode_pt opc);
static opsize_pt displsize(struct ea_s *eap);
static reg_pt fpregchk(void);
static bool_pt getaccumreg(struct ea_s *eap);
static void getbinary(void);
static bool_pt getdxreg(struct ea_s *eap);
static void getea(struct ea_s *eap);
static void getimmed(struct ea_s *eap, count_t immed_count);
static void getindirect(struct ea_s *eap);
static void getshift(struct ea_s *eap);
static reg_pt indregchk(reg_pt matchreg);
static void kgerror(char * err_str);
static void lbranch(int backamount);
static void notbytesize(struct ea_s *eap);
static void notimmed(struct ea_s *eap);
static void notindirect(struct ea_s *eap);
static void notsegorspecreg(struct ea_s *eap);
static void yesimmed(struct ea_s *eap);
static void yes_samesize(void);

static void Eb(eap)
register struct ea_s *eap;
{
    Ex(eap);
    if (eap->size != 0x1)
    {
#ifndef NODEFAULTSIZE
	if (eap->size == 0x0)
	    eap->size = 0x1;
	else
#endif
	    kgerror(ILL_SIZE);
    }
}

static void Ew(eap)
register struct ea_s *eap;
{
    Ex(eap);
    if (eap->size != 0x2)
    {
#ifndef NODEFAULTSIZE
	if (eap->size == 0x0)
	    eap->size = 0x2;
	else
#endif
	    kgerror(ILL_SIZE);
    }
}

static void Ev(eap)
register struct ea_s *eap;
{
    Ex(eap);
    notbytesize(eap);
}

static void Ex(eap)
register struct ea_s *eap;
{
    getea(eap);
    notimmed(eap);
    notsegorspecreg(eap);
}

static void Gd(eap)
register struct ea_s *eap;
{
    Gx(eap);
    if (eap->size != 0x4)
	kgerror(ILL_SIZE);
}

static void Gw(eap)
register struct ea_s *eap;
{
    Gx(eap);
    if (eap->size != 0x2)
	kgerror(ILL_SIZE);
}

static void Gv(eap)
register struct ea_s *eap;
{
    Gx(eap);
    notbytesize(eap);
}

static void Gx(eap)
register struct ea_s *eap;
{
    Ex(eap);
    notindirect(eap);
}

static void buildea(eap)
register struct ea_s *eap;
{
    opsize_t asize;

    ++mcount;
    lastexp = eap->displ;
    if (eap->indcount == 0x0)
	postb = REG_MOD | rm[eap->base];
    else
    {
	if (eap->base == NOREG)
	{
	    if (eap->index == NOREG)
	    {
		if ((asize = displsize(eap)) > 0x2)
		    postb = D32_RM;
		else
		    postb = D16_RM;
	    }
	    else
	    {
		asize = 0x4;
		postb = SIB_NOBASE;	/* for sib later */
	    }
	}
	else
	{
	    if (eap->base > MAX16BITINDREG)
	    {
		asize = 0x4;
		postb = rm[eap->base];
	    }
	    else
	    {
		asize = 0x2;
		if (!(lastexp.data & UNDBIT) &&
		    !iswordorswordoffset(lastexp.offset))
		    error(ABOUNDS);
		if (eap->index == NOREG)
		    postb = rmfunny[eap->base];
		else if (eap->base <= MAX16BITINDREG)
		    postb = baseind16[eap->base + 0x4 * eap->index];
	    }
	}
	needcpu(asize==4?3:0);
	if (asize != defsize)
	    aprefix = 0x67;
	if (eap->base == NOREG)
	    mcount += asize;
	else if (lastexp.data & (FORBIT | RELBIT | UNDBIT) ||
		 !is8bitsignedoffset(lastexp.offset))
	{
	    postb |= MEM2_MOD;
	    mcount += asize;
	}
	else if (lastexp.offset != 0x0 ||
		 (eap->base == BPREG && eap->index == NOREG) ||
		 eap->base == EBPREG)
	{
	    postb |= MEM1_MOD;
	    ++mcount;
	}
	if (asize > 0x2 && (eap->base == ESPREG || eap->index != NOREG))
	{
	    sib = ss[eap->scale] |
		(rm[eap->index] << INDEX_SHIFT) |
		(postb & RM_MASK);
	    postb = (postb & MOD_MASK) | SIB_RM;
	    ++mcount;
	}
    }
}

static void buildfloat(void)
{
    if (mcount != 0x0)
    {
	buildea(&source);
	oprefix = 0x0;
	postb |= (opcode & 0x07) << REG_SHIFT;
	opcode = ESCAPE_OPCODE_BASE | ((opcode & 0x70) >> 0x4);
    }
}

static void buildfreg(void)
{
    mcount += 0x2;
    oprefix = 0x0;
    postb = REG_MOD | ((opcode & 0x07) << REG_SHIFT) | (target.base - ST0REG);
    opcode = ESCAPE_OPCODE_BASE | ((opcode & 0x70) >> 0x4);
}

static void buildimm(eap, signflag)
register struct ea_s *eap;
bool_pt signflag;
{
    immadr = eap->displ;
    immcount = eap->size;
    if (!(immadr.data & (FORBIT | RELBIT | UNDBIT)))
    {
	if (immcount == 0x1)
	{
	    if ((offset_t) (immadr.offset + 0x80) >= 0x180)
		datatoobig();
	}
	else if (signflag && is8bitsignedoffset(immadr.offset))
	{
	    opcode |= SIGNBIT;
	    immcount = 0x1;
	}
	else if (immcount == 0x2)
	{
	    if ((offset_t) (immadr.offset + 0x8000L) >= 0x18000L)
		datatoobig();
	}
    }
}

static void buildregular(void)
{
    if (mcount != 0x0)
    {
	buildea(&target);
	postb |= regbits[source.base];
    }
}

/* Check size and build segword. */

static void buildsegword(eap)
register struct ea_s *eap;
{
    if (eap->size == 0x0)
#ifdef NODEFAULTSIZE
	kgerror(SIZE_UNK);
#else
	eap->size = defsize;
#endif
    if (eap->indcount != 0x0 || eap->base == NOREG)
    {
	segword = WORDBIT;
	if (eap->size == 0x1)
	    segword = BYTE_SEGWORD;
    }
    else
	segword = regsegword[eap->base];
}

static void buildunary(opc)
opcode_pt opc;
{
    if (mcount != 0x0)
    {
	buildea(&target);
	postb |= opcode;
	opcode = opc;
    }
}

static opsize_pt displsize(eap)
register struct ea_s *eap;
{
    opsize_t asize;

    asize = defsize;
    if (!(eap->displ.data & UNDBIT))
    {
	if (asize > 0x2)
	{
	    if (!(eap->displ.data & (FORBIT | RELBIT)) &&
		iswordadr(eap->displ.offset))
		asize = 0x2;
	}
	else if (!iswordorswordoffset(eap->displ.offset))
				/* should really use iswordadr() */
				/* but compiler generates signed offsets */
	{
	    if (!(eap->displ.data & (FORBIT | RELBIT)))
		asize = 0x4;
	    else if (pass2)
		error(ABOUNDS);
	}
    }
    return asize;
}

static reg_pt fpregchk(void)
{
    reg_pt fpreg;

    fpreg_allowed = TRUE;
    fpreg = regchk();
    fpreg_allowed = FALSE;
    if (fpreg != ST0REG)
	return NOREG;
    getsym();
    if (sym == LPAREN)
    {
	getsym();
	if (sym != INTCONST || (unsigned) number >= 0x8)
	    error(ILL_FP_REG);
	else
	{
	    fpreg += number;
	    getsym();
	    if (sym != RPAREN)
		error(RPEXP);
	    else
		getsym();
	}
    }
    return fpreg;
}

static bool_pt getaccumreg(eap)
register struct ea_s *eap;
{
    if ((eap->base = regchk()) != AXREG && eap->base != ALREG
	&& eap->base != EAXREG)
	return FALSE;
    getsym();
    if ((eap->size = regsize[eap->base]) > 0x1 && eap->size != defsize)
	oprefix = 0x66;
    return TRUE;
}

/*
  Get binary ea's in target & source (flipped if direction is set).
  Put size in source if not already.
  Initialise direction, segword, bump mcount.
*/

static void getbinary(void)
{
    ++mcount;
    getea(&target);
    if (target.indcount == 0x0 && target.base == NOREG)
    {
	error(ILL_IMM_MODE);
	target.base = AXREG;
	target.size = defsize;
    }
    getcomma();
    getea(&source);
    if (source.size == 0x0)
	source.size = target.size;
    else if (target.size != 0x0 && target.size != source.size)
    {
	kgerror(MISMATCHED_SIZE);
	return;
    }
    if (source.indcount == 0x0 && regsegword[target.base] < SEGMOV)
	direction = 0x0;
    else if (target.indcount == 0x0 && regsegword[source.base] < SEGMOV)
    {
	struct ea_s swap;

	direction = TOREGBIT;
	swap = source;
	source = target;
	target = swap;
    }
    else if (target.indcount != 0x0)
    {
	kgerror(ILL_IND_TO_IND);
	return;
    }
    else
    {
	kgerror(ILL_SEG_REG);
	return;
    }
    buildsegword(&source);
}

static bool_pt getdxreg(eap)
register struct ea_s *eap;
{
    if ((eap->base = regchk()) != DXREG)
	return FALSE;
    getsym();
    return TRUE;
}

/* parse effective address */

/*
  Syntax is restrictive in that displacements must be in the right spots
  and will not be added up.

  optional size-type prefix, which is
    BYTE
    BYTE PTR
    WORD
    WORD PTR
    DWORD
    DWORD PTR
    PTR
  reg
  segreg
  [scaled index]
  where scaled index =
    indreg
    indreg*scale
    indreg+indreg
    indreg+indreg*scale
  [scaled index+displ]
  [scaled index-displ]
  optional-immediate-prefix displ[scaled index]
  [displ]
  optional-imediate-prefix displ
  (scaled index)                                   -- anachronism
  optional-imediate-prefix displ(scaled index)     -- anachronism
*/

static void getea(eap)
register struct ea_s *eap;
{
    bool_t leading_displ;
    bool_t leading_immed;
    register struct sym_s *symptr;

    leading_immed = leading_displ = lastexp.data = eap->indcount
		  = lastexp.offset = 0x0;
    eap->index = eap->base = NOREG;
    eap->scale = 0x1;
    eap->size = mnsize;		/* 0x1 for byte ops, else 0x0 */

    if (sym == IDENT)
    {
	if ((symptr = gsymptr)->type & MNREGBIT)
	{
	    if (symptr->data & SIZEBIT)
	    {
		getsym();
		if (symptr->value_reg_or_op.op.opcode == 0x0)
		    eap->indcount = 0x2 - calljmp_kludge;
		else
		{
		    if (eap->size != 0x0)
		    {
			if (eap->size != symptr->value_reg_or_op.op.opcode)
			    error(MISMATCHED_SIZE);
		    }
		    else
			eap->size = symptr->value_reg_or_op.op.opcode;
		    if (eap->size > 0x1 && eap->size != defsize)
			oprefix = 0x66;
		    if (sym == IDENT &&
			(symptr = gsymptr)->type & MNREGBIT &&
			symptr->data & SIZEBIT &&
			symptr->value_reg_or_op.op.routine == PTROP)
		    {
			getsym();
			eap->indcount = 0x2 - calljmp_kludge;
		    }
		}
	    }
	}
	if( last_pass == 1 )
	    if (!(symptr->type & (LABIT | MACBIT | MNREGBIT | VARBIT)))
	        symptr->data |= FORBIT;	/* show seen in advance */
    }
    if ((eap->base = regchk()) != NOREG)
    {
	getsym();
	if (eap->indcount != 0x0)
	{
	    error(ILL_IND_PTR);
	    eap->indcount = 0x0;
	}
	if (eap->size != 0x0 && eap->size != regsize[eap->base])
	    error(MISMATCHED_SIZE);
	if ((eap->size = regsize[eap->base]) > 0x1 && eap->size != defsize)
	    oprefix = 0x66;
	eap->displ = lastexp;
        needcpu(eap->size==4?3:0);
	return;
    }
    if (sym != lindirect)
    {
	if (sym == IMMEDIATE || sym == STAR)
	{
	    /* context-sensitive, STAR means signed immediate here */
	    leading_immed = TRUE;
	    getsym();
	}
	leading_displ = TRUE;
	expres();
	eap->displ = lastexp;
    }
    if (sym == lindirect)
    {
	getsym();
	eap->indcount = 0x2 - calljmp_kludge;
	if ((eap->base = indregchk((reg_pt) NOREG)) != NOREG)
	{
	    if (eap->indcount == 0x0 && leading_displ)
		error(IND_REQ);
	    getsym();
	    if (sym == ADDOP)
	    {
		getsym();
		if ((eap->index = indregchk(eap->base)) != NOREG)
		    getsym();
		else
		{
		    if (eap->indcount == 0x0)
			error(IND_REQ);
		    if (leading_displ)
			error(REPEATED_DISPL);
		    expres();	/* this eats ADDOP, SUBOP, MULOP */
		}
	    }
	    if (sym == STAR)
	    {
                needcpu(3);
		/* context-sensitive, STAR means scaled here*/
		if (eap->index == NOREG && eap->base == ESPREG)
		{
		    error(INDEX_REG_EXP);
		    eap->base = EAXREG;
		}
		getsym();
		factor();
		chkabs();
		if (!(lastexp.data & UNDBIT) && lastexp.offset != 0x1)
		{
		    if (eap->base <= MAX16BITINDREG ||
			(lastexp.offset != 0x2 && lastexp.offset != 0x4 &&
			lastexp.offset != 0x8))
			error(ILL_SCALE);
		    else
		    {
			eap->scale = lastexp.offset;
			if (eap->index == NOREG)
			{
			    eap->index = eap->base;
			    eap->base = NOREG;
			}
		    }
		}
		lastexp.data = lastexp.offset = 0x0;
	    }
	    if ((sym == ADDOP || sym == SUBOP))
	    {
		if (eap->indcount == 0x0)
		    error(IND_REQ);
		if (leading_displ)
		    error(REPEATED_DISPL);
		expres();
	    }
	}
	else
	{
	    if (leading_displ)
		error(REPEATED_DISPL);
	    expres();
	}
	if (sym != rindirect)
	    error(rindexp);
	else
	    getsym();
    }
    /* RDB */
    else if (!leading_immed && defsize <= 0x2)
	eap->indcount = 0x1;	/* compatibility kludge */
    if (!leading_displ)
	eap->displ = lastexp;

    needcpu(eap->size==4?3:0);
}

static void getimmed(eap, immed_count)
struct ea_s *eap; 
count_t immed_count;
{
    getea(eap);
    yesimmed(eap);
    if (mcount != 0x0)
    {
	eap->size = immed_count;
	buildimm(eap, FALSE);
    }
}

static void getindirect(eap)
register struct ea_s *eap;
{
    getea(eap);
    if (eap->indcount == 0x0)
	kgerror(IND_REQ);
}

static void getshift(eap)
register struct ea_s *eap;
{
    getcomma();
    getea(eap);
    if (eap->base != CLREG)
	yesimmed(eap);
}

/*
  Check if current symbol is a compatible index register.
  Generate error if it is a reg but not a compatible index.
  Return register number (adjusted if necessary to a legal index) or NOREG.
*/

static reg_pt indregchk(matchreg)
reg_pt matchreg;
{
    reg_pt reg;

    if ((reg = regchk()) != NOREG)
    {
	switch (matchreg)
	{
	case BPREG:
	case BXREG:
	    if (reg != DIREG && reg != SIREG)
	    {
		reg = SIREG;
		error(INDEX_REG_EXP);
	    }
	    break;
	case DIREG:
	case SIREG:
	    if (reg != BPREG && reg != BXREG)
	    {
		reg = BXREG;
		error(INDEX_REG_EXP);
	    }
	    break;
	case NOREG:
	    break;
	default:
	    if (reg <= MAX16BITINDREG || reg == ESPREG)
	    {
		reg = EAXREG;
		error(INDEX_REG_EXP);
	    }
	    break;
	}
	if (reg > MAXINDREG && calljmp_kludge == 0x0)
	{
	    if (matchreg != NOREG)
		reg = EAXREG;
	    else
		reg = BXREG;
	    error(INDEX_REG_EXP);
	}
    }
    return reg;
}

static void kgerror(err_str)
char * err_str;
{
    error(err_str);
    sprefix = oprefix = aprefix = mcount = 0x0;
}

static void lbranch(backamount)
int backamount;
{
    mcount += defsize + 0x1;
    segadj();
    if (pass2)
    {
	reldata();
	if (!(lastexp.data & (RELBIT | UNDBIT)))
	{
	    lastexp.offset = lastexp.offset - lc - lcjump;
	    if ( last_pass<2 && backamount != 0x0 && 
	        !(lastexp.data & IMPBIT) &&
		lastexp.offset + backamount < 0x80 + backamount)
		warning(SHORTB);	/* -0x8? to 0x7F, warning */
	}
    }
}

/* BCC (long branches emulated by short branch over & long jump) */

void mbcc(void)
{
    getea(&target);
    if (target.indcount >= 0x2 || target.base != NOREG)
	kgerror(REL_REQ);
    else
    {
#ifdef iscpu
	if (iscpu(3))
#else
	if (defsize != 0x2)
#endif
	{
	    page = PAGE1_OPCODE;
	    ++mcount;
	    opcode += 0x10;
	    lbranch(0x84);
	}
	else
	{
	    aprefix = opcode ^ 0x1;	/* kludged storage for short branch
					   over */
	    oprefix = defsize + 0x1;
	    mcount += 0x2;
	    opcode = JMP_OPCODE;
	    lbranch(0x83);
	    mcount -= 0x2;
	}
    }
}

/* bswap r32 */

void mbswap(void)
{
    needcpu(4);
    ++mcount;
    Gd(&target);
    opcode |= rm[target.base];
}

/* BR, CALL, J, JMP */

void mcall(void)
{
    opcode_pt far_diff;
    bool_t indirect;
    register struct sym_s *symptr;

    far_diff = 0x0;
    if (sym == IDENT && (symptr = gsymptr)->type & MNREGBIT &&
	symptr->data & SIZEBIT )
    {
        if(symptr->value_reg_or_op.op.routine == FAROP)
        {
	    far_diff = 0x8;
	    getsym();
        }
        if(symptr->value_reg_or_op.op.routine == WORDOP &&
	   opcode == JMP_SHORT_OPCODE)
        {
	   opcode = JMP_OPCODE;
	   getsym();
	}
    }
    indirect = FALSE;

    if (asld_compatible && defsize <= 0x2)
    {
	calljmp_kludge = 0x2;
	if (sym == INDIRECT)
	{
	    calljmp_kludge = 0x0;
	    indirect = TRUE;
	    getsym();
	}
    }
    getea(&target);
    if (indirect && target.indcount == 0x1)
	target.indcount = 0x2;
    calljmp_kludge = 0x0;
    if (sym == COLON)
    {
        int tsize = target.size?target.size:defsize;
	if (opcode == JMP_SHORT_OPCODE)
	    opcode = JMP_OPCODE;
	++mcount;
	yesimmed(&target);
	getsym();
	getea(&source);
	yesimmed(&source);
	if (mcount != 0x0)
	{
	    if (opcode == JMP_OPCODE)
		opcode = 0xEA;
	    else
		opcode = 0x9A;
	    lastexp = source.displ;
	    if (!(lastexp.data & (FORBIT | RELBIT | UNDBIT)) &&
		tsize == 0x2 &&
		(offset_t) (lastexp.offset + 0x8000L) >= 0x18000L)
		datatoobig();
	    mcount += tsize;
	    target.size = 0x2;
	    buildimm(&target, FALSE);
	}
    }
    else if (target.indcount >= 0x2 || target.base != NOREG)
    {
	++mcount;
	notsegorspecreg(&target);
	if (target.indcount == 0)
	    notbytesize(&target);
	if (mcount != 0x0)
	{
	    if (opcode == JMP_SHORT_OPCODE)
		opcode = JMP_OPCODE;
	    buildea(&target);
	    if (opcode == JMP_OPCODE)
		opcode = 0x20;
	    else
		opcode = 0x10;
	    postb |= opcode + far_diff;
	    opcode = 0xFF;
	}
    }
    else if (opcode == JMP_SHORT_OPCODE)
    {
	if (jumps_long &&
	    ((pass!=0 && !is8bitsignedoffset(lastexp.offset - lc - 2)) ||
	     (last_pass==1)))
	{
	    opcode = JMP_OPCODE;
	    lbranch(0x83);
	}
	else
	{
	    lastexp = target.displ;
	    if (lastexp.data & IMPBIT)
	    {
		error(NONIMPREQ);
	        lastexp.data = FORBIT | UNDBIT;
    	    }
    	    mshort2();
	}
    }
    else
	lbranch(opcode == JMP_OPCODE ? 0x83 : 0x0);
}

/* CALLI, JMPI */

void mcalli(void)
{
    bool_t indirect;

    ++mcount;
    indirect = FALSE;
    if (sym == INDIRECT)
    {
	getsym();
	indirect = TRUE;
    }
    getea(&target);
    if (target.indcount >= 0x2 || target.base != NOREG)
	indirect = TRUE;
    if (indirect)
    {
	buildea(&target);
	if (opcode == 0xEA)
	    opcode = 0x28;
	else
	    opcode = 0x18;
	postb |= opcode;
	opcode = 0xFF;
    }
    else
    {
        int tsize = target.size?target.size:defsize;
	getcomma();
	getea(&source);
	yesimmed(&source);
	if (mcount != 0x0)
	{
	    lastexp = target.displ;
	    if (!(lastexp.data & (FORBIT | RELBIT | UNDBIT)) &&
		tsize == 0x2 &&
		(offset_t) (lastexp.offset + 0x8000L) >= 0x18000L)
            {
		tsize=4;
	        if( tsize != defsize ) oprefix = 0x66;
		/* datatoobig(); */
	    }
	    needcpu(tsize==4?3:0);
	    mcount += tsize;
	    source.size = 0x2;
	    buildimm(&source, FALSE);
	}
    }
}

/* DIV, IDIV, MUL */

void mdivmul(void)
{
    if (getaccumreg(&source))
    {
	++mcount;
	getcomma();
	Ex(&target);
	yes_samesize();
	buildunary(0xF6 | regsegword[source.base]);
    }
    else
	mnegnot();
}

/* ENTER */

void menter(void)
{
    ++mcount;
    getimmed(&target, 0x2);
    getcomma();
    getimmed(&source, 0x1);
    if (mcount != 0x0)
    {
	mcount += 2;
	lastexp = target.displ;	/* getimmed(&source) wiped it out */
    }
    needcpu(1);
}

/* arpl r/m16,r16 (Intel manual opcode chart wrongly says EwRw) */

void mEwGw(void)
{
    ++mcount;
    Ew(&target);
    getcomma();
    Gw(&source);
    oprefix = 0x0;
    buildregular();
}

/* [cmpxchg xadd] [r/m8,r8 r/m16,r16, r/m32,r32] */

void mExGx(void)
{
    ++mcount;
    Ex(&target);
    getcomma();
    Gx(&source);
    yes_samesize();
    opcode |= segword;
    buildregular();
}

void mf_inher(void)
{
    mcount += 0x2;
    postb = REG_MOD | (opcode & ~REG_MOD);
    opcode = ESCAPE_OPCODE_BASE | (opcode >> 0x6);
    if (opcode == ESCAPE_OPCODE_BASE)
	opcode = ESCAPE_OPCODE_BASE | 0x6;	/* fix up encoding of fcompp */
}

/* [fldenv fnsave fnstenv frstor] mem */

void mf_m(void)
{
    ++mcount;
    getindirect(&source);
    if (source.size != 0x0)
	kgerror(ILL_SIZE);
    buildfloat();
}

/* [fldcw fnstcw] mem2i */

void mf_m2(void)
{
    ++mcount;
    getindirect(&source);
    if (source.size != 0x0 && source.size != 0x2)
	kgerror(ILL_SIZE);
    buildfloat();
}

/* fnstsw [mem2i ax] */

void mf_m2_ax(void)
{
    if (getaccumreg(&target))
    {
	if (target.base != AXREG)
	    kgerror(ILLREG);
	else
	{
	    opcode = 0x74;
	    target.base = ST0REG;	/* fake, really ax */
	    buildfreg();
	}
    }
    else
	mf_m2();
}

/* [fiadd ficom ficomp fidiv fidivr fimul fist fisub fisubr] [mem2i mem4i] */

void mf_m2_m4(void)
{
    ++mcount;
    getindirect(&source);
    if (source.size == 0x0)
	kgerror(SIZE_UNK);
    else if (source.size == 0x2)
	opcode |= 0x40;
    else if (source.size != 0x4)
	kgerror(ILL_SIZE);
    buildfloat();
}

/* [fild fistp] [mem2i mem4i mem8i] */

void mf_m2_m4_m8(void)
{
    ++mcount;
    getindirect(&source);
    if (source.size == 0x0)
	kgerror(SIZE_UNK);
    else if (source.size == 0x2)
	opcode |= 0x40;
    else if (source.size == 0x8)
	opcode |= 0x45;		/* low bits 0 -> 5 and 3 -> 7 */
    else if (source.size != 0x4)
	kgerror(ILL_SIZE);
    buildfloat();
}

/* [fcom fcomp] [mem4r mem8r optional-st(i)] */

void mf_m4_m8_optst(void)
{
    if (sym == EOLSYM)
    {
	target.base = ST1REG;
	buildfreg();
    }
    else
	mf_m4_m8_st();
}

/* [fadd fdiv fdivr fmul fsub fsubr] [mem4r mem8r st,st(i) st(i),st] */

void mf_m4_m8_stst(void)
{
    target.base = fpregchk();
    if (target.base != NOREG)
    {
	getcomma();
	source.base = fpregchk();
	if (source.base == NOREG)
	{
	    error(FP_REG_REQ);
	    source.base = ST0REG;
	}
	if (target.base == ST0REG)
	    target.base = source.base;
	else
	{
	    if (source.base != ST0REG)
		error(ILL_FP_REG_PAIR);
	    opcode |= 0x40;
	    if ((opcode & 0x07) >= 0x4)
		opcode ^= 0x01;	/* weird swap of fdiv/fdivr, fsub/fsubr */
	}
	buildfreg();
    }
    else
    {
	++mcount;
	getindirect(&source);
	if (source.size == 0x0)
	    kgerror(SIZE_UNK);
	else if (source.size == 0x8)
	    opcode |= 0x40;
	else if (source.size != 0x4)
	    kgerror(ILL_SIZE);
	buildfloat();
    }
}

/* fst [mem4r mem8r st(i)] */

void mf_m4_m8_st(void)
{
    target.base = fpregchk();
    if (target.base != NOREG)
    {
	if (opcode == FST_ENCODED)
	    opcode |= 0x40;
	buildfreg();
    }
    else
    {
	++mcount;
	getindirect(&source);
	if (source.size == 0x0)
	    kgerror(SIZE_UNK);
	else if (source.size == 0x8)
	    opcode |= 0x40;
	else if (source.size != 0x4)
	    kgerror(ILL_SIZE);
	buildfloat();
    }
}

/* [fld fstp] [mem4r mem8r mem10r st(i)] */

void mf_m4_m8_m10_st(void)
{
    target.base = fpregchk();
    if (target.base != NOREG)
    {
	if (opcode == FSTP_ENCODED)
	    opcode |= 0x40;
	buildfreg();
    }
    else
    {
	++mcount;
	getindirect(&source);
	if (source.size == 0x0)
	    kgerror(SIZE_UNK);
	else if (source.size == 0x8)
	    opcode |= 0x40;
	else if (source.size == 0xA)
	    opcode |= 0x25;	/* low bits 0 -> 5 and 3 -> 7 */
	else if (source.size != 0x4)
	    kgerror(ILL_SIZE);
	buildfloat();
    }
}

/* [fbld fbstp] mem10r */

void mf_m10(void)
{
    ++mcount;
    getindirect(&source);
    if (source.size != 0xA)
	kgerror(ILL_SIZE);
    buildfloat();
}

/* ffree st(i) */

void mf_st(void)
{
    target.base = fpregchk();
    if (target.base == NOREG)
	kgerror(FP_REG_REQ);
    buildfreg();
}

/* [fucom fucomp fxch] optional-st(i) */

void mf_optst(void)
{
    if (sym == EOLSYM)
    {
	target.base = ST1REG;
	buildfreg();
    }
    else
	mf_st();
}

/* [faddp fdivp fdivrp fmulp fsubp fsubrp] st(i),st */

void mf_stst(void)
{
    target.base = fpregchk();
    if (target.base == NOREG)
    {
	kgerror(FP_REG_REQ);
	return;
    }
    getcomma();
    source.base = fpregchk();
    if (source.base == NOREG)
    {
	kgerror(FP_REG_REQ);
	return;
    }
    if (source.base != ST0REG)
    {
	kgerror(ILL_FP_REG);
	return;
    }
    buildfreg();
}

void mf_w_inher(void)
{
    sprefix = WAIT_OPCODE;
    mf_inher();
}

/* [fsave fstenv] mem */

void mf_w_m(void)
{
    sprefix = WAIT_OPCODE;
    mf_m();
}

/* fstcw mem2i */

void mf_w_m2(void)
{
    sprefix = WAIT_OPCODE;
    mf_m2();
}

/* fstsw [mem2i ax] */

void mf_w_m2_ax(void)
{
    sprefix = WAIT_OPCODE;
    mf_m2_ax();
}

/* ADC, ADD, AND, CMP, OR, SBB, SUB, XOR */

void mgroup1(void)
{
    getbinary();
    notsegorspecreg(&source);
    if (mcount != 0x0)
    {
	if (source.base == NOREG)
	{
	    if (target.indcount == 0x0 && (target.base == ALREG ||
					   target.base == AXREG ||
					  (target.base == EAXREG &&
			  (source.displ.data & (FORBIT | RELBIT | UNDBIT) ||
			   !is8bitsignedoffset(source.displ.offset)))))
	    {
		opcode |= 0x04 | segword;
		buildimm(&source, FALSE);
	    }
	    else
	    {
		buildunary(0x80 | segword);
		buildimm(&source, TRUE);
	    }
	}
	else
	{
	    opcode |= direction | segword;
	    buildregular();
	}
    }
}

/* RCL, RCR, ROL, ROR, SAL, SAR, SHL, SHR */

void mgroup2(void)
{
    ++mcount;
    Ex(&target);
    buildsegword(&target);
    getshift(&source);
    if (mcount != 0x0)
    {
	buildunary(0xD0 | segword);
	if (source.base == CLREG)
	    opcode |= 0x2;
	else if (source.displ.offset != 0x1)
	{
	    needcpu(1);
	    opcode -= 0x10;
	    source.size = 0x1;
	    buildimm(&source, FALSE);
	}
    }
}

/* LLDT, LTR, SLDT, STR, VERR, VERW */

void mgroup6(void)
{
    needcpu(2);
    ++mcount;
    Ew(&target);
    oprefix = 0x0;
    buildunary(0x0);
}

/* INVLPG, LGDT, LIDT, LMSW, SGDT, SIDT, SMSW */

void mgroup7(void)
{
    needcpu(2);	/* I think INVLPG is actually 386 */
    ++mcount;
    if (opcode == 0x20 || opcode == 0x30)
    {
	Ew(&target);
	oprefix = 0x0;
    }
    else
    {
	getindirect(&target);
	oprefix = 0x0;
	if (target.size != 0x0 && target.size != 0x6)
	    error(MISMATCHED_SIZE);	/* XXX - size 6 wrong for INVLPG? */
    }
    buildunary(0x1);
}

/* BT, BTR, BTS, BTC */

void mgroup8(void)
{
    needcpu(3);
    ++mcount;
    Ev(&target);
    getcomma();
    /* Gv or Ib */
    getea(&source);
    notindirect(&source);
    notsegorspecreg(&source);
    if (mcount != 0x0)
    {
	if (source.base == NOREG)
	{
	    buildunary(0xBA);
	    source.size = 0x1;
	    buildimm(&source, TRUE);
	}
	else
	{
	    yes_samesize();
	    opcode += 0x83;
	    buildregular();
	}
    }
}

/* BSF, BSR, LAR, LSL (Intel manual opcode chart wrongly says GvEw for L*) */

void mGvEv(void)
{
    needcpu(2);
    ++mcount;
    Gv(&source);
    getcomma();
    Ev(&target);
    yes_samesize();
    buildregular();
}

/* bound [r16,m16&16 r32,m32&32] */

void mGvMa(void)
{
    ++mcount;
    Gv(&source);
    getcomma();
    getindirect(&target);
    yes_samesize();
    buildregular();
}

/* LDS, LES, LFS, LGS, LSS */

void mGvMp(void)
{
    ++mcount;
    Gv(&source);
    getcomma();
    getindirect(&target);
    if (target.size != 0x0 && target.size != 0x2 + source.size)
	error(MISMATCHED_SIZE);
    buildregular();
}

/* IMUL */

void mimul(void)
{
    ++mcount;
    Ex(&target);
    if (sym != COMMA)
    {
	buildsegword(&target);
	buildunary(0xF6 | segword);
	return;
    }
    getcomma();
    notindirect(&target);
    source = target;		/* direction is swapped */
    getea(&target);
    notsegorspecreg(&target);
    yes_samesize();
    if (sym != COMMA && (target.indcount != 0x0 || target.base != NOREG))
    {
        needcpu(3);
	page = PAGE1_OPCODE;
	++mcount;
	opcode = 0xAF;
	buildregular();
    }
    else
    {
	if (sym == COMMA)
	{
	    getsym();
	    getea(&source2);
	    yesimmed(&source2);
	}
	else
	{
	    source2 = target;
	    target = source;
	}
	source2.size = target.size;
	if (is8bitsignedoffset(source2.displ.offset))
	{
	    source2.size = 0x1;
	    opcode = 0x6B;
	}
	else
	{
	    source2.size = target.size;
	    opcode = 0x69;
	}
	buildregular();
	if (mcount != 0x0)
	    buildimm(&source2, FALSE);
    }
}

/* IN */

void min(void)
{
    ++mcount;
    if (opcode & WORDBIT)	/* inw; ind not supported */
	mnsize = 0x2;
    if (sym == EOLSYM && mnsize != 0x0)
	    target.size = mnsize;
    else
    {
	if (getaccumreg(&target))
	{
	    if (mnsize != 0x0 && regsize[target.base] != mnsize)
		error(MISMATCHED_SIZE);
	    getcomma();
	}
	else
	    target.size = regsize[target.base = mnsize < 0x2 ? ALREG : AXREG];
	opcode |= regsegword[target.base];
	if (!getdxreg(&source))
	{
	    getimmed(&source, 0x1);
	    opcode -= 0x8;
	}
    }
    if (target.size > 0x1 && target.size != defsize)
	oprefix = 0x66;
}

/* DEC, INC */

void mincdec(void)
{
    ++mcount;
    Ex(&target);
    buildsegword(&target);
    if (target.indcount == 0x0 && segword == WORDBIT)
	opcode |= 0x40 | rm[target.base];
    else
	buildunary(0xFE | segword);
}

/* CBW, CWD, CMPSW, INSW, IRET, LODSW, POPA, POPF, PUSHA, PUSHF */
/* MOVSW, OUTSW, SCASW, STOSW */

void minher16(void)
{
    minher();
    if (defsize != 0x2)
	oprefix = 0x66;
}

/* CWDE, CDQ, CMPSD, INSD, IRETD, LODSD, POPAD, POPFD, PUSHAD, PUSHFD */
/* MOVSD, OUTSD, SCASD, STOSD */

void minher32(void)
{
    minher();
    if (defsize != 0x4)
	oprefix = 0x66;
    needcpu(3);
}

/* AAD, AAM */

void minhera(void)
{
    ++mcount;
    if (sym == EOLSYM)
    {
	target.displ.offset = 0xA;
	target.size = 0x1;
	buildimm(&target, FALSE);
    }
    else
	getimmed(&target, 0x1);
}

/* INT */

void mint(void)
{
    ++mcount;
    getimmed(&target, 0x1);
    if (!(immadr.data & (FORBIT | RELBIT | UNDBIT)) &&
	(opcode_t) immadr.offset == 0x3)
    {
	immcount = 0x0;
	opcode = 0xCC;
    }
}

/* JCC */

void mjcc(void)
{
    /* First look for j* near */
    if (sym == IDENT && 
        gsymptr->type & MNREGBIT &&
	gsymptr->data & SIZEBIT &&
        gsymptr->value_reg_or_op.op.routine == WORDOP &&
	opcode < 0x80)
    {
        getsym();
        getea(&target);
        if (target.indcount >= 0x2 || target.base != NOREG)
 	   kgerror(REL_REQ);
        else
        {
            needcpu(3);
	    page = PAGE1_OPCODE;
	    ++mcount;
	    opcode += 0x10;
	    lbranch(0x84);
        }
    }
    else if (!jumps_long || opcode > 0x80) /* above 0x80 means loop, not long */
	mshort();
    else  /* mbcc */
    {
        getea(&target);
	lastexp = target.displ;

	if ( (pass!=0 && !is8bitsignedoffset(lastexp.offset - lc - 2)) ||
	      last_pass==1)
	{
           if (target.indcount >= 0x2 || target.base != NOREG)
	       kgerror(REL_REQ);

	    aprefix = opcode ^ 0x1;	/* kludged storage for short branch
					   over */
	    oprefix = defsize + 0x1;
	    mcount += 0x2;
	    opcode = JMP_OPCODE;
	    lbranch(0x83);
	    mcount -= 0x2;
	}
	else
	{
	   /* 8 bit */
	    if (lastexp.data & IMPBIT)
	    {
		error(NONIMPREQ);
		lastexp.data = FORBIT | UNDBIT;
    	    }
    	    mshort2();
	}
    }
}

/* JCXZ, JECXZ */

void mjcxz(void)
{
    if (opcode != defsize)
    {
	aprefix = 0x67;
	++mcount;		/* quick fix - mshort() needs to know */
    }
    opcode = 0xE3;
    mshort();
    if (aprefix != 0x0)
	--mcount;		/* quick fix - main routine bumps it again */
}

/* LEA */

void mlea(void)
{
    Gv(&source);		/* back to front */
    getcomma();
    ++mcount;
    getindirect(&target);
    yes_samesize();
    buildregular();
}

/* MOV */

void mmov(void)
{
    getbinary();
    if (segword >= SEGMOV)
    {
	oprefix = 0x0;
	notimmed(&target);	/* target is actually the source */
	if (segword > SEGMOV)	/* special reg */
	    notindirect(&target);
    }
    if (mcount != 0x0)
    {
	if (target.base == NOREG && target.index == NOREG &&
	    (source.base == ALREG || source.base == AXREG ||
	     source.base == EAXREG))
	{
	    opcode = 0xA0 | (direction ^ TOREGBIT) | segword;
	    lastexp = target.displ;
	    if ((source.size = displsize(&target)) != defsize)
		aprefix = 0x67;
	    mcount += source.size;
	    needcpu(source.size==4?3:0);
	}
	else if (source.base == NOREG)
	{
	    if (target.indcount == 0x0)
		opcode = 0xB0 | (segword << 0x3) | rm[target.base];
	    else
	    {
		buildea(&target);
		opcode = 0xC6 | segword;
	    }
	    buildimm(&source, FALSE);
	}
	else
	{
	    if (isspecreg(source.base))
	    {
		needcpu(3);
		page = PAGE1_OPCODE;
		++mcount;
		opcode = 0x0;
	    }
	    opcode |= direction | segword;
	    buildregular();
	}
    }
}

/* MOVSX, MOVZX */

void mmovx(void)
{
    ++mcount;
    Gv(&source);
    getcomma();
    Ex(&target);
    if (target.size == 0x0)
	kgerror(SIZE_UNK);
    if (target.size > 0x2)
	kgerror(ILL_SIZE);
    oprefix = 0x0;
    if (source.size != defsize)
	oprefix = 0x66;
    buildsegword(&target);
    opcode |= segword;
    buildregular();
}

/* NEG, NOT */

void mnegnot(void)
{
    ++mcount;
    Ex(&target);
    buildsegword(&target);
    buildunary(0xF6 | segword);
}

/* OUT */

void mout(void)
{
    ++mcount;
    if (opcode & WORDBIT)	/* outw; outd not supported */
	mnsize = 0x2;
    if (sym == EOLSYM && mnsize != 0x0)
	    source.size = mnsize;
    else
    {
	if (!getdxreg(&target))
	{
	    getimmed(&target, 0x1);
	    opcode -= 0x8;
	}
	if (sym == COMMA)
	{
	    getsym();
	    if (!getaccumreg(&source))
		kgerror(AL_AX_EAX_EXP);
	    else if (mnsize != 0x0 && regsize[source.base] != mnsize)
		error(MISMATCHED_SIZE);
	}
	else
	    source.size = regsize[source.base = mnsize < 0x2 ? ALREG : AXREG];
	opcode |= regsegword[source.base];
    }
    if (source.size > 0x1 && source.size != defsize)
	oprefix = 0x66;
}

/* POP, PUSH */

void mpushpop(void)
{
    opcode_t oldopcode;

    ++mcount;
    getea(&target);
    buildsegword(&target);
    notbytesize(&target);
    if ((oldopcode = opcode) == POP_OPCODE)
    {
	notimmed(&target);
	if (target.base == CSREG)
	    kgerror(ILL_SEG_REG);
    }
    if (mcount != 0x0)
    {
	if (target.indcount == 0x0)
	{
	    if (segword == SEGMOV)
	    {
		switch (target.base)
		{
		case CSREG:
		    opcode = 0x0E;
		    break;
		case DSREG:
		    opcode = 0x1E;
		    break;
		case ESREG:
		    opcode = 0x06;
		    break;
		case SSREG:
		    opcode = 0x16;
		    break;
		case FSREG:
		    opcode = 0xA0;
		    page = PAGE1_OPCODE;
		    ++mcount;
		    break;
		case GSREG:
		    opcode = 0xA8;
		    page = PAGE1_OPCODE;
		    ++mcount;
		    break;
		}
		if (oldopcode == POP_OPCODE)
		    ++opcode;
	    }
	    else if (target.base != NOREG)
	    {
		opcode = 0x50 | rm[target.base];
		if (oldopcode == POP_OPCODE)
		    opcode |= 0x8;
	    }
	    else
	    {
		needcpu(1); /* On 8086 PUSH does not allow immediate */
		opcode = 0x68;
		if (oldopcode == POP_OPCODE)
		    ++opcode;
		buildimm(&target, TRUE);
	    }
	}
	else
	{
	    buildea(&target);
	    if (oldopcode == PUSH_OPCODE)
		postb |= 0x6 << REG_SHIFT;
	}
    }
}

/* RET, RETF */

void mret(void)
{
    ++mcount;
    if (sym != EOLSYM)
    {
	--opcode;
	getimmed(&target, 0x2);
    }
}

/* SEG CS/DS/ES/FS/GS/SS */

void mseg(void)
{
    reg_pt reg;

    if (regsegword[reg = regchk()] != SEGMOV)
	error(SEG_REG_REQ);
    else
    {
	getsym();
	++mcount;
	opcode = (segoverride - CSREG)[reg];
    }
}

/* SETCC */

void msetcc(void)
{
    ++mcount;
    Eb(&target);
    if (mcount != 0x0)
	buildea(&target);
}

/* SHLD, SHRD */

void mshdouble(void)
{
    needcpu(3);
    ++mcount;
    Ev(&target);
    getcomma();
    Gv(&source);
    yes_samesize();
    buildregular();
    getshift(&source2);
    lastexp = target.displ;	/* getshift() wiped it out */
    if (mcount != 0x0)
    {
	if (source2.base == CLREG)
	    opcode |= 0x1;
	else
	{
	    source2.size = 0x1;
	    buildimm(&source2, FALSE);
	}
    }
}

/*
  TEST
  Similar to the regular group1 operators.
  It does not allow the sign extended immediate byte forms
  and does not use the relevant direction bit.
*/

void mtest(void)
{
    getbinary();
    notsegorspecreg(&source);
    if (source.base == NOREG)
    {
	if (mcount != 0x0)
	{
	    if (target.indcount == 0x0
		&& (target.base == ALREG || target.base == AXREG
		    || target.base == EAXREG))
		opcode = 0xA8 | segword;
	    else
	    {
		buildea(&target);
		opcode = 0xF6 | segword;
	    }
	}
	buildimm(&source, FALSE);
    }
    else
    {
	opcode |= segword;
	buildregular();
    }
}

/*
  XCHG
  Similar to the regular group1 operators.
  It does not allow any of the immediate forms
  and does not use the irrelevant direction bit.
*/

void mxchg(void)
{
    getbinary();
    notimmed(&source);
    notsegorspecreg(&source);
    if (target.indcount == 0x0)
    {
	if (target.base == AXREG || target.base == EAXREG)
	{
	    opcode = 0x90 + rm[source.base];
	    return;
	}
	if (source.base == AXREG || source.base == EAXREG)
	{
	    opcode = 0x90 + rm[target.base];
	    return;
	}
    }
    opcode |= segword;
    buildregular();
}

static void notbytesize(eap)
register struct ea_s *eap;
{
    if (eap->size == 0x1)
	kgerror(ILL_SIZE);
}

static void notimmed(eap)
register struct ea_s *eap;
{
    if (eap->indcount == 0x0 && eap->base == NOREG)
	kgerror(ILL_IMM_MODE);
}

static void notindirect(eap)
register struct ea_s *eap;
{
    if (eap->indcount != 0x0)
	kgerror(ILL_IND);
}

static void notsegorspecreg(eap)
register struct ea_s *eap;
{
    if (regsegword[eap->base] >= SEGMOV)
	kgerror(ILLREG);
}

static void yesimmed(eap)
register struct ea_s *eap;
{
    if (eap->indcount == 0x1)
	eap->indcount = 0x0;
    if (eap->indcount != 0x0 || eap->base != NOREG)
	kgerror(IMM_REQ);
}

static void yes_samesize(void)
{
    if (target.size == 0x0)
	target.size = source.size;
    else if (source.size != 0x0 && target.size != source.size)
	kgerror(MISMATCHED_SIZE);
}

#endif /* I80386 */

#ifdef MC6809

/* 6809 opcode constants */

/* bits for indexed addressing */

#define INDIRECTBIT 0x10
#define INDEXBIT    0x80	/* except 5 bit offset */
#define PCRELBIT    0x04	/* PC relative (in certain cases) */
#define RRBITS      0x60	/* register select bits */

static opcode_t rrindex[] =	/* register and index bits for indexed adr */
{
    0x60 | INDEXBIT,		/* S */
    0x40 | INDEXBIT,		/* U */
    0x00 | INDEXBIT,		/* X */
    0x20 | INDEXBIT,		/* Y */
    PCRELBIT | INDEXBIT,	/* PC */
};

static opcode_t pushpull[] =	/* push/pull codes  */
{
    0x40,			/* S */
    0x40,			/* U */
    0x10,			/* X */
    0x20,			/* Y */
    0x80,			/* PC */
    0x02,			/* A */
    0x04,			/* B */
    0x01,			/* CC */
    0x08,			/* DP */
    0x06,			/* D */
};

static opcode_t tfrexg1[] =	/* transfer/exchange codes for source reg */
{
    0x40,			/* S */
    0x30,			/* U */
    0x10,			/* X */
    0x20,			/* Y */
    0x50,			/* PC */
    0x80,			/* A */
    0x90,			/* B */
    0xA0,			/* CC */
    0xB0,			/* DP */
    0x00,			/* D */
};

static opcode_t tfrexg2[] =	/* transfer/exchange codes for target reg */
{
    0x04,			/* S */
    0x03,			/* U */
    0x01,			/* X */
    0x02,			/* Y */
    0x05,			/* PC */
    0x08,			/* A */
    0x09,			/* B */
    0x0A,			/* CC */
    0x0B,			/* DP */
    0x00,			/* D */
};

static void checkpostinc(void);
static void doaltind(void);
static void do1altind(void);
static void fixupind(void);
static void getindexnopost(void);
static void inderror(char * err_str);
static reg_pt indreg(reg_pt maxindex);
static void predec1(void);
static void sustack(reg_pt stackreg);

void checkpostinc(void)
{
    if (sym == ADDOP)
    {
	if (postb & INDIRECTBIT)
	    inderror(ILLMOD);	/* single-inc indirect illegal */
	else
	{
	    lastexp.offset &= 0xFF00;	/* for printing if postbyte is 0: ,X+ */
	    getsym();
	}
    }
    else if (sym == POSTINCOP)
    {
	postb |= 0x1;
	getsym();
    }
    else
	postb |= 0x4;
    fixupind();
}

/* common code for all-mode ops, alterable-mode ops, indexed ops */

void doaltind(void)
{
    mcount += 0x2;
    if (sym == LBRACKET)
    {
	postb = INDIRECTBIT;
	getsym();
	do1altind();
	if (sym != RBRACKET)
	    error(RBEXP);
    }
    else
	do1altind();
}

void do1altind(void)
{
    bool_t byteflag;		/* set if direct or short indexed adr forced */
    char *oldlineptr;
    char *oldsymname;
    reg_pt reg;
    bool_t wordflag;		/* set if extended or long indexed adr forced*/

    if ((reg = regchk()) != NOREG)
    {
	switch (reg)
	{
	case AREG:
	    postb |= 0x86;
	    break;
	case BREG:
	    postb |= 0x85;
	    break;
	case DREG:
	    postb |= 0x8B;
	    break;
	default:
	    if (indreg(MAXINDREG) != NOREG)
		checkpostinc();
	    return;
	}
	getsym();
	if (sym != COMMA)
	    inderror(COMEXP);
	else
	    getindexnopost();
	return;
    }
    else if (sym == SUBOP)	/* could be -R or - in expression */
    {
	oldlineptr = lineptr;	/* save state */
	oldsymname = symname;
	getsym();
	reg = regchk();
	lineptr = oldlineptr;
	symname = oldsymname;
	if (reg != NOREG)
	{
	    predec1();		/* it's -R */
	    return;
	}
	sym = SUBOP;
    }
    else if (sym == COMMA)
    {
	postb |= INDEXBIT;
	getsym();
	if (sym == SUBOP)
	{
	    predec1();
	    return;
	}
	else if (sym != PREDECOP)
	{
	    if (indreg(MAXINDREG) != NOREG)
		checkpostinc();
	    return;
	}
    }
    if (sym == PREDECOP)
    {
	postb |= 0x83;
	getindexnopost();
	return;
    }

    /* should have expression */

    wordflag = byteflag = FALSE;
    if (sym == LESSTHAN)
    {
	/* context-sensitive, LESSTHAN means byte-sized here */
	byteflag = TRUE;
	getsym();
    }
    else if (sym == GREATERTHAN)
    {
	/* context-sensitive, GREATERTHAN means word-sized here */
	wordflag = TRUE;
	getsym();
    }
    expres();
    if (sym == COMMA)
    {				/* offset from register */
	getsym();
	if ((reg = indreg(PCREG)) == NOREG)
	    return;
	postb |= 0x8;		/* default 8 bit offset */
	if (reg == PCREG)
	{
	    reldata();
	    if (!(lastexp.data & (RELBIT | UNDBIT)))
	    {
		lastexp.offset = lastexp.offset - lc;
		if (page != 0x0)
		    lastexp.offset -= 0x4;	/* extra for instruction */
		else
		    lastexp.offset -= 0x3;	/* 3 byte instruction
						   assuming 8 bit offset */
	    }
	}
	if (byteflag)
	{
	    if (!(lastexp.data & (RELBIT | UNDBIT)) &&
		!is8bitsignedoffset(lastexp.offset))
		error(ABOUNDS);	/* forced short form is impossible */
	    ++mcount;
	}
	else if (wordflag || lastexp.data & (FORBIT | RELBIT | UNDBIT) ||
		 !is8bitsignedoffset(lastexp.offset))
	{			/* 16 bit offset */
	    if (postb & PCRELBIT && !(lastexp.data & RELBIT))
		--lastexp.offset;	/* instruction 1 longer than already
					   allowed */
	    postb |= 0x1;
	    mcount += 0x2;
	}
	else if (!(postb & PCRELBIT) &&
		 (offset_t) (lastexp.offset + 0x10) < 0x20 &&
		 !(postb & INDIRECTBIT && lastexp.offset != 0x0))
	{			/* 5 bit offset */
	    postb &= RRBITS | INDIRECTBIT;
	    if (lastexp.offset == 0x0)
		postb |= 0x84;	/* index with zero offset */
	    else
		postb |= (lastexp.offset & 0x1F);
	}
	else			/* 8 bit offset */
	    ++mcount;
	fixupind();
    }
    else if (postb & INDIRECTBIT)
    {				/* extended indirect */
	postb = 0x9F;
	mcount += 0x2;
	fixupind();
    }
    else if (postb & INDEXBIT)
	inderror(ILLMOD);	/* e.g. LEAX	 $10 */
    else
    {
	if (byteflag || (!wordflag && !(lastexp.data & (FORBIT | RELBIT)) &&
	    (lastexp.offset >> 0x8) == dirpag))
	{			/* direct addressing */
	    if (opcode >= 0x80)
		opcode |= 0x10;
	}
	else			/* extended addressing */
	{
	    if (opcode < 0x80)
		opcode |= 0x70;
	    else
		opcode |= 0x30;
	    ++mcount;
	    if (pass2 && (opcode == JSR_OPCODE || opcode == JMP_OPCODE) &&
		!(lastexp.data & IMPBIT) &&
		lastexp.offset + (0x81 - 0x3) < 0x101)
				/* JSR or JMP could be done with BSR or BRA */
		warning(SHORTB);
	}
    }
}

void fixupind(void)
{
    if ((opcode & 0x30) == 0x0)	/* change all but LEA opcodes */
    {
	if (opcode < 0x80)
	    opcode |= 0x60;
	else
	    opcode |= 0x20;
    }
}

void getindexnopost(void)
{
    getsym();
    if (indreg(MAXINDREG) != NOREG)
	fixupind();
}

void inderror(char *err_str)
{
    error(err_str);
    if (postb & INDIRECTBIT)
	sym = RBRACKET;		/* fake right bracket to kill further errors */
    fixupind();
}

/* check current symbol is an index register (possibly excepting PC) */
/* if so, modify postbyte RR and INDEXBIT for it, get next sym, return TRUE */
/* otherwise generate error, return FALSE */

reg_pt indreg(reg_pt maxindex)
{
    reg_pt reg;

    if ((reg = regchk()) == NOREG)
	inderror(IREGEXP);
    else if (reg > maxindex)
    {
	inderror(ILLREG);
	reg = NOREG;
    }
    else
    {
	postb |= rrindex[reg];
	getsym();
    }
    return reg;
}

/* all-mode ops */

void mall(void)
{
    if (sym == IMMEDIATE)
	mimmed();
    else
	malter();
}

/* alterable mode ops */

void malter(void)
{
    postb = 0x0;		/* not yet indexed or indirect */
    doaltind();
}

/* indexed mode ops */

void mindex(void)
{
    postb = INDEXBIT;		/* indexed but not yet indirect */
    doaltind();
}

/* immediate ops */

void mimmed(void)
{
    opcode_t nybble;

    mcount += 0x2;
    if (sym != IMMEDIATE)
	error(ILLMOD);
    else
    {
	if (opcode >= 0x80 && ((nybble = opcode & 0xF) == 0x3 ||
			       nybble == 0xC || nybble >= 0xE))
	    ++mcount;		/* magic for long immediate */
	symexpres();
	if (pass2 && mcount <= 0x2)
	{
	    chkabs();
	    checkdatabounds();
	}
    }
}

/* long branches */

void mlong(void)
{
    mcount += 0x3;		/* may be 0x0 or 0x1 here */
    expres();
    segadj();
    if (pass2)
    {
	reldata();
	if (!(lastexp.data & (RELBIT | UNDBIT)))
	{
	    lastexp.offset = lastexp.offset - lc - lcjump;
	    if ( last_pass<2 && !(lastexp.data & IMPBIT) &&
		lastexp.offset + 0x81 < 0x101)
		warning(SHORTB);	/* -0x81 to 0x7F, warning */
	}
    }
}

/* PSHS and PULS */

void msstak(void)
{
    sustack(SREG);
}

/* TFR and EXG */

void mswap(void)
{
    reg_pt reg;

    mcount = 0x2;
    if ((reg = regchk()) == NOREG)
	error(REGEXP);
    else
    {
	postb = tfrexg1[reg];
	getsym();
	if (sym != COMMA)
	    error(COMEXP);
	else
	{
	    getsym();
	    if ((reg = regchk()) == NOREG)
		error(REGEXP);
	    else if ((postb |= tfrexg2[reg])
		     & 0x88 && (postb & 0x88) != 0x88)
		error(ILLREG);	/* registers not of same size */
	}
    }
    getsym();
}

/* PSHU and PULU */

void mustak(void)
{
    sustack(UREG);
}

void predec1(void)
{
    if (postb & INDIRECTBIT)
	inderror(ILLMOD);	/* single-dec indirect illegal */
    else
    {
	postb |= 0x82;
	getindexnopost();
    }
}

/* common routine for PSHS/PULS/PSHU/PULU */

void sustack(reg_pt stackreg)
{
    reg_pt reg;

    mcount = 0x2;
    while ((reg = regchk()) != NOREG)
    {
	if (reg == stackreg)
	{
	    error(ILLREG);	/* cannot stack self */
	    break;
	}
	postb |= pushpull[reg];
	getsym();
	if (sym != COMMA)
	    break;
	getsym();
    }
}

#endif /* MC6809 */

/* routines common to all processors */

void getcomma(void)
{
    if (sym != COMMA)
	error(COMEXP);
    else
	getsym();
}

/* inherent ops */

/* for I80386 */
/* AAA, AAS, CLC, CLD, CLI, CLTS, CMC, CMPSB, DAA, DAS, HLT, INTO, INSB, */
/* INVD, */
/* LAHF, LEAVE, LOCK, LODSB, MOVSB, NOP, OUTSB, REP, REPE, REPNE, REPNZ, */
/* REPZ, SAHF, SCASB, STC, STD, STI, STOSB, WAIT, WBINVD */

void minher(void)
{
    ++mcount;
}

/* short branches */

void mshort(void)
{
    nonimpexpres();
    mshort2();
}

void mshort2(void)
{
    mcount += 0x2;
    if (pass2)
    {
	reldata();
	if (lastexp.data & RELBIT)
	    showrelbad();
	else if (!(lastexp.data & UNDBIT))
	{
	    lastexp.offset = lastexp.offset - lc - mcount;
	    if (!is8bitsignedoffset(lastexp.offset))
		error(ABOUNDS);
	}
    }
}

/* check if current symbol is a register, return register number or NOREG */

reg_pt regchk(void)
{
    register struct sym_s *symptr;

    if (sym == IDENT)
    {
	if ((symptr = gsymptr)->type & MNREGBIT)
	{
	    if (symptr->data & REGBIT)
	    {
	        int regno = symptr->value_reg_or_op.reg;
#ifdef I80386
		if (regno == ST0REG && !fpreg_allowed)
		    error(FP_REG_NOT_ALLOWED);

		/* Check cpu */
                needcpu((regno==FSREG||regno==GSREG)?3:0);
                needcpu((regno>=EAXREG && regno<=ESPREG)?3:0);
                needcpu((regno>=CR0REG && regno<=TR7REG)?3:0);
#endif
		return regno;
	    }
	}
	else
	    if( last_pass == 1 )
	        if (!(symptr->type & (LABIT | MACBIT | VARBIT)))
	            symptr->data |= FORBIT;	/* show seen in advance */
    }
    return NOREG;
}

/* convert lastexp.data for PC relative */

void reldata(void)
{
    if ((lastexp.data ^ lcdata) & (IMPBIT | RELBIT | SEGM))
    {
	if ((lastexp.data ^ lcdata) & RELBIT)
	    showrelbad();	/* rel - abs is weird, abs - rel is bad */
	else
	{
	    pcrflag = OBJ_R_MASK;
	    lastexp.data = (lcdata & ~SEGM) | lastexp.data | RELBIT;
				/* segment is that of lastexp.data */
	}
    }
    else			/* same file, segment and relocation */
	lastexp.data = (lastexp.data | lcdata) & ~(RELBIT | SEGM);
}

void segadj(void)
{
    if ((lastexp.data & UNDBIT) && textseg >= 0 )
    {
        lastexp.sym->data &= ~SEGM;
        lastexp.sym->data |= (lcdata & SEGM);
    }
}
