/*
 * Z-80 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 *
 * FIXME: why is ld (ix) producing broken relocs while (ix + 0) is ok ??
 */
#include	"as.h"

#define	OPDJNZ	0x10			/* Opcode: djnz */
#define	OPADD	0x80			/* Opcode: add */
#define	OPDAD	0x09			/* Opcode: dad */
#define	OPADC	0x88			/* Opcode: adc */
#define	OPADCW	0x4A			/* Opcode: adc hl */
#define	OPSBCW	0x42			/* Opcode: sbc hl */
#define	OPSUBI	0xC6			/* Opcode: make immediate */
#define	OPXCHG	0xEB			/* Opcode: xchg */
#define	OPXTHL	0xE3			/* Opcode: xthl */
#define	OPEXAF	0x08			/* Opcode: ex af,af' */
#define	OPRST	0xC7			/* Opcode: rst 0 */
#define	OPINCRP	0x03			/* Opcode: inc rp */
#define	OPDECRP	0x0B			/* Opcode: dec rp */
#define	OPINC	0x04			/* Opcode: inc */
#define	OPIIN	0x40			/* Opcode: indirect in */
#define	OPIOUT	0x41			/* Opcode: indirect out */
#define	OPIN	0xDB			/* Opcode: in */
#define	OPIM	0x46			/* Opcode: im */
#define	OPPCHL	0xE9			/* Opcode: jp (hl) */
#define	OPJP	0xC3			/* Opcode: jp cc base */
#define	OPJR	0x20			/* Opcode: jr cc base */
#define	OPRET	0xC0			/* Opcode: ret cc base */

static void asmld(void);
static ADDR *getldaddr(ADDR *ap, int *modep, int *regp, ADDR *iap);
static void outop(int op, ADDR *ap);
static int ccfetch(ADDR *ap);

/*
 * CPU specific pass setup
 */

static int cputype;
/* FIXME: we should malloc/realloc this on non 8bit machines */
static uint8_t reltab[1024];
static unsigned int nextrel;

int passbegin(int pass)
{
	cputype = 80;
	segment = 1;		/* Default to code */
	if (pass == 3)
		nextrel = 0;
	return 1;		/* All passes required */
}

static void setnextrel(int flag)
{
	if (nextrel == 8 * sizeof(reltab))
		aerr(TOO_MANY_JR);
	if (flag)
		reltab[nextrel >> 3] |= (1 << (nextrel & 7));
	nextrel++;
}

static unsigned int getnextrel(void)
{
	unsigned int n = reltab[nextrel >> 3] & (1 << (nextrel & 7));
	nextrel++;
	return n;
}


static void require_z180(void)
{
	if (!(cpu_flags & OA_8080_Z180)) {
		cpu_flags |= OA_8080_Z180;
		if (cputype != 180)
			err('1',REQUIRE_Z180);
	}
}

static unsigned jr_to_jp(unsigned opcode)
{
	/* DJNZ - we don't turn this into inc/jp as it's not quite the same */
	if (opcode == OPDJNZ) {
		aerr(BRA_RANGE);
		return opcode;
	}
	/* JR to JP */
	if (opcode == 0x18)
		return 0xC3;
	/* JR cc - extract relevant bits  and add them to the JP c base */
	opcode &= 0x38;
	return 0xC2|opcode;
}

/*
 * Read in an address
 * descriptor, and fill in
 * the supplied "ADDR" structure
 * with the mode and value.
 * Exits directly to "qerr" if
 * there is no address field or
 * if the syntax is bad.
 */
void getaddr(ADDR *ap)
{
	int reg;
	int c;

	ap->a_flags = 0;
	ap->a_sym = NULL;

	if ((c=getnb()) != '(') {
		if (c == '<')
			ap->a_flags |= A_LOW;
		else if (c == '>')
			ap->a_flags |= A_HIGH;
		else if (c != '#')
			unget(c);
		expr1(ap, LOPRI, 0);
		return;
	}
	expr1(ap, LOPRI, 1);
	if (getnb() != ')')
		qerr(BRACKET_EXPECTED);
	reg = ap->a_type&TMREG;
	switch (ap->a_type&TMMODE) {
	case TBR:
		if (reg != C)
			aerr(REG_MUST_BE_C);
		ap->a_type |= TMINDIR;
		break;
	case TSR:
	case TCC:
		aerr(ADDR_REQUIRED);
		break;
	case TUSER:
		ap->a_type |= TMINDIR;
		break;
	case TWR:
		if (reg == HL)
			ap->a_type = TBR|M;
		else if (reg==IX || reg==IY)
			ap->a_type = TBR|reg;
		else if (reg==AF || reg==AFPRIME)
			aerr(INVALID_REG);
		else
			ap->a_type |= TMINDIR;
	}
}

/* Little endian */
static void outaw(uint16_t a)
{
	outab(a);
	outab(a >> 8);
}

/*
 * Assemble one line.
 * The line in in "ib", the "ip"
 * scans along it. The code is written
 * right out.
 */
void asmline(void)
{
	SYM *sp;
	int c;
	int opcode;
	int disp;
	int reg;
	int srcreg;
	int cc;
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;
	ADDR a2;

loop:
	if ((c=getnb())=='\n' || c==';')
		return;
	if (isalpha(c) == 0 && c != '_' && c != '.')
		qerr(UNEXPECTED_CHR);
	getid(id, c);
	if ((c=getnb()) == ':') {
		sp = lookup(id, uhash, 1);
		/* Pass 0 we compute the worst cases
		   Pass 1 we generate according to those 
		   Pass 2 we set them in stone (the shrinkage from pass 1
					        allowing us a lot more)
		   Pass 3 we output accodingly */
		if (pass == 0) {
			if ((sp->s_type&TMMODE) != TNEW
			&&  (sp->s_type&TMASG) == 0)
				sp->s_type |= TMMDF;
			sp->s_type &= ~TMMODE;
			sp->s_type |= TUSER;
			sp->s_value = dot[segment];
			sp->s_segment = segment;
		} else if (pass != 3) {
			/* Don't check for duplicates, we did it already
			   and we will confuse ourselves with the pass
			   before. Instead blindly update the values */
			sp->s_type &= ~TMMODE;
			sp->s_type |= TUSER;
			sp->s_value = dot[segment];
			sp->s_segment = segment;
		} else {
			/* Phase 2 defined the values so a misalignment here
			   is fatal */
			if ((sp->s_type&TMMDF) != 0)
				err('m', MULTIPLE_DEFS);
			if (sp->s_value != dot[segment])
				err('p', PHASE_ERROR);
		}
		goto loop;
	}
	/*
	 * If the first token is an
	 * id and not an operation code,
	 * assume that it is the name in front
	 * of an "equ" assembler directive.
	 */
	if ((sp=lookup(id, phash, 0)) == NULL) {
		getid(id1, c);
		if ((sp1=lookup(id1, phash, 0)) == NULL
		||  (sp1->s_type&TMMODE) != TEQU) {
			err('o', SYNTAX_ERROR);
			return;
		}
		getaddr(&a1);
		istuser(&a1);
		sp = lookup(id, uhash, 1);
		if ((sp->s_type&TMMODE) != TNEW
		&&  (sp->s_type&TMASG) == 0)
			err('m', MULTIPLE_DEFS);
		sp->s_type &= ~(TMMODE|TPUBLIC);
		sp->s_type |= TUSER|TMASG;
		sp->s_value = a1.a_value;
		sp->s_segment = a1.a_segment;
		/* FIXME: review .equ to an external symbol/offset and
		   what should happen */
		goto loop;
	}
	unget(c);
	opcode = sp->s_value;
	switch (sp->s_type&TMMODE) {
	case TORG:
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_segment != ABSOLUTE)
			qerr(MUST_BE_ABSOLUTE);
		outsegment(ABSOLUTE);
		dot[segment] = a1.a_value;
		/* Tell the binary generator we've got a new absolute
		   segment. */
		outabsolute(a1.a_value);
		break;

	case TEXPORT:
		getid(id, getnb());
		sp = lookup(id, uhash, 1);
		sp->s_type |= TPUBLIC;
		break;
		/* .code etc */
	case TSEGMENT:
		segment = sp->s_value;
		/* Tell the binary generator about a segment switch to a non
		   absolute segnent */
		outsegment(segment);
		break;

	case TDEFB:
		do {
			getaddr(&a1);
			istuser(&a1);
			outrab(&a1);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFW:
		do {
			getaddr(&a1);
			istuser(&a1);
			outraw(&a1);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFM:
		if ((delim=getnb()) == '\n')
			qerr(MISSING_DELIMITER);
		while ((c=get()) != delim) {
			if (c == '\n')
				qerr(MISSING_DELIMITER);
			outab(c);
		}
		break;

	case TDEFS:
		getaddr(&a1);
		istuser(&a1);
		/* Write out the bytes. The BSS will deal with the rest */
		for (value = 0 ; value < a1.a_value; value++)
			outab(0);
		break;

	case TSETCPU:
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value != 180 && a1.a_value != 80)
			aerr(SYNTAX_ERROR);
		cputype = a1.a_value;
		break;

	case TNOP180:
		require_z180();
		/* Fall through */
	case TNOP:
		if ((opcode&0xFF00) != 0)
			outab(opcode >> 8);
		outab(opcode);
		break;

	case TRST:
		getaddr(&a1);
		istuser(&a1);
		if ((a1.a_value & 7) || a1.a_value < 0 || a1.a_value > 0x38)
			aerr(INVALID_CONST);
		else
			outab(OPRST|a1.a_value);
		break;

	case TREL:
		getaddr(&a1);
		if ((cc=ccfetch(&a1)) >= 0) {
			if (opcode==OPDJNZ || cc>=CPO)
				aerr(SYNTAX_ERROR);
			opcode = OPJR | (cc<<3);
			comma();
			getaddr(&a1);
		}
		istuser(&a1);
		disp = a1.a_value-dot[segment]-2;
		if (pass == 3)
			c = getnextrel();
		else {
			c = 0;
			if (pass == 0 || a1.a_segment != segment || disp < -128 || disp > 127)
				c = 1;
			/* On pass 2 we lock down our choice in the table */
			if (pass == 2)
				setnextrel(c);
		}
		if (c) {
			opcode = jr_to_jp(opcode);
			outab(opcode);
			outraw(&a1);
		} else {
			/* These shoudl never happen */
			if (a1.a_segment == UNKNOWN)
				aerr(UNKNOWN_SYMBOL);
			else if (disp<-128 || disp>127 || a1.a_segment != segment)
				aerr(BRA_RANGE);
			/* Write the relative form */
			outab(opcode);
			outab(disp);
		}
		break;

	case TRET:
		unget(c = getnb());
		if (c!='\n' && c!=';') {
			getaddr(&a1);
			if ((cc=ccfetch(&a1)) < 0)
				aerr(CONDCODE_ONLY);
			opcode = OPRET | (cc<<3);
		}
		outab(opcode);
		break;

	case TJMP:
		getaddr(&a1);
		if ((cc=ccfetch(&a1)) >= 0) {
			opcode = (opcode&0x00C6) | (cc<<3);
			comma();
			getaddr(&a1);
		}
		if ((a1.a_type&TMMODE) == TBR) {
			reg = a1.a_type&TMREG;
			if (reg==M || reg==IX || reg==IY) {
				if (opcode != OPJP)
					aerr(INVALID_REG);
				outop(OPPCHL, &a1);
				break;
			}
		}
		istuser(&a1);
		outab(opcode);
		outraw(&a1);
		break;

	case TPUSH:
		getaddr(&a1);
		if ((a1.a_type&TMMODE) == TWR) {
			reg = a1.a_type&TMREG;
			switch (reg) {
			case IX:
				outab(0xDD);
				reg = HL;
				break;
			case IY:
				outab(0xFD);
				reg = HL;
				break;
			case AF:
				reg = SP;
				break;
			case SP:
			case AFPRIME:
				aerr(INVALID_REG);
			}
			outab(opcode|(reg<<4));
			break;
		}
		aerr(INVALID_REG);
		break;

	case TIM:
		getaddr(&a1);
		istuser(&a1);
		if ((value=a1.a_value) > 2)
			aerr(INVALID_CONST);
		else if (value != 0)
			++value;
		outab(0xED);
		outab(OPIM|(value<<3));
		break;

	case TIMMED8:
		require_z180();
		getaddr(&a1);
		istuser(&a1);
		outab(opcode);
		outabchk2(value);
		break;

	case TIO180:	/* out0 and in0 */
		require_z180();
		getaddr((opcode & 1) ? &a2 : &a1);
		comma();
		getaddr((opcode & 1) ? &a1 : &a2);
		if ((a1.a_type&TMMODE) == TBR && a2.a_type == (TUSER|TMINDIR)) {
			reg = a1.a_type & TMREG;
			if (reg == M || reg == IX || reg == IY)
				aerr(INVALID_REG);
			outab(opcode << 8);
			outab(opcode | (reg << 3));
			outabchk2(a2.a_value);
			break;
		}
		aerr(INVALID_REG);

	case TIO:
		getaddr(opcode==OPIN ? &a1 : &a2);
		comma();
		getaddr(opcode==OPIN ? &a2 : &a1);
		if (a1.a_type==(TBR|A) && a2.a_type==(TUSER|TMINDIR)) {
			outab(opcode);
			outabchk2(a2.a_value);
			break;
		}
		if ((a1.a_type&TMMODE)==TBR && a2.a_type==(TBR|TMINDIR|C)) {
			reg = a1.a_type&TMREG;
			if (reg==M || reg==IX || reg==IY)
				aerr(INVALID_REG);
			outab(0xED);
			if (opcode == OPIN)
				opcode = OPIIN; else
				opcode = OPIOUT;
			outab(opcode|(reg<<3));
			break;
		}
		aerr(INVALID_REG);
		break;

	case TBIT:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if ((a1.a_type&TMMODE) == TUSER
		&&   a1.a_value < 8
		&&  (a2.a_type&TMMODE) == TBR) {
			if ((reg=a2.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|(a1.a_value<<3)|reg, &a2);
			break;
		}
		aerr(INVALID_REG);
		break;

	case TSHR:
		getaddr(&a1);
		if ((a1.a_type&TMMODE) == TBR) {
			if ((reg=a1.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|reg, &a1);
			break;
		}
		aerr(INVALID_REG);

	case TINC:
		getaddr(&a1);
		if ((a1.a_type&TMMODE) == TWR) {
			reg = a1.a_type&TMREG;
			switch (reg) {
			case IX:
				outab(0xDD);
				reg = HL;
				break;
			case IY:
				outab(0xFD);
				reg = HL;
				break;
			case AF:
			case AFPRIME:
				aerr(INVALID_REG);
			}
			if (opcode == OPINC)
				opcode = OPINCRP; else
				opcode = OPDECRP;
			outab(opcode|(reg<<4));
			break;
		}
		if ((a1.a_type&TMMODE) == TBR) {
			if ((reg=a1.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|(reg<<3), &a1);
			break;
		}
		aerr(INVALID_REG);
		break;

	case TEX:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if (a1.a_type==(TWR|AF) && a2.a_type==(TWR|AFPRIME)) {
			outab(OPEXAF);
			break;
		}
		if (a1.a_type == (TWR|DE))
			opcode = OPXCHG;
		else if (a1.a_type == (TWR|TMINDIR|SP))
			opcode = OPXTHL;
		else
			aerr(INVALID_REG);
		if (a2.a_type == (TWR|HL))
			outab(opcode);
		else if (a2.a_type == (TWR|IX)) {
			outab(0xDD);
			outab(opcode);
		} else if (a2.a_type == (TWR|IY)) {
			outab(0xFD);
			outab(opcode);
		} else
			aerr(INVALID_REG);
		break;

	case TMUL:
		getaddr(&a1);
		if ((a1.a_type & TMMODE) == TWR) {
			reg = a1.a_type & TMREG;
			switch(reg) {
				case IX:
				case IY:
				case AF:
				case AFPRIME:
					aerr(INVALID_REG);
					break;
			}
			outab(opcode >> 8);
			outab(opcode|(reg << 3));
			break;
		}
		aerr(INVALID_REG);
		break;

	case TSUB:
		getaddr(&a1);
		if (a1.a_type == TUSER) {
			outab(opcode | OPSUBI);
			outabchk2(a1.a_value);
			break;
		}
		if ((a1.a_type&TMMODE) == TBR) {
			if ((reg=a1.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|reg, &a1);
			break;
		}
		aerr(INVALID_REG);
		break;

	case TADD:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if (a1.a_type == (TBR|A)) {
			if (a2.a_type == TUSER) {
				outab(opcode | OPSUBI);
				outabchk2(a2.a_value);
				break;
			}
			if ((a2.a_type&TMMODE) == TBR) {
				if ((reg=a2.a_type&TMREG)==IX || reg==IY)
					reg = M;
				outop(opcode|reg, &a1);
				break;
			}
		}
		if ((a1.a_type&TMMODE) == TWR) {
			switch(reg = a1.a_type&TMREG) {
			case IX:
				if (opcode != OPADD)
					aerr(INVALID_REG);
				outab(0xDD);
				opcode = OPDAD;
				srcreg = IX;
				break;
			case IY:
				if (opcode != OPADD)
					aerr(INVALID_REG);
				outab(0xFD);
				opcode = OPDAD;
				srcreg = IY;
				break;
			case HL:
				if (opcode == OPADD)
					opcode = OPDAD;
				else {
					outab(0xED);
					if (opcode == OPADC)
						opcode = OPADCW;
					else
						opcode = OPSBCW;
				}
				srcreg = HL;
				break;
			default:
				aerr(INVALID_REG);
			}
			if ((a2.a_type&TMMODE) == TWR) {
				reg = a2.a_type&TMREG;
				if (reg==BC || reg==DE || reg==SP) {
					outab(opcode|(reg<<4));
					break;
				}
				if (reg == srcreg) {
					outab(opcode|(HL<<4));
					break;
				}
			}
		}
		aerr(INVALID_REG);
		break;

	case TLD:
		asmld();
		break;

	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}

/*
 * Handle the dreaded "ld" instruction,
 * with its dozens of forms, formats and special
 * encodings. The "getldaddr" routine performs most
 * of the special stuff for index registers and for
 * indexing. This layer just screens out the many
 * cases, and emits the correct bytes.
 */
static void asmld(void)
{
	int mdst;
	int rdst;
	int msrc;
	int rsrc;
	ADDR *indexap;
	ADDR dst;
	ADDR src;

	indexap = NULL;
	indexap = getldaddr(&dst, &mdst, &rdst, indexap);
	comma();
	indexap = getldaddr(&src, &msrc, &rsrc, indexap);
	if (dst.a_type == (TBR|A)) {
		if (msrc == TSR) {
			if (rsrc == I)
				outaw(0x57ED);		/* ld a,i */
			else
				outaw(0x5FED);		/* ld a,r */
			return;
		}
		if (msrc == (TMINDIR|TUSER)) {
			outab(0x3A);			/* ld a,(addr) */
			outraw(&src);
			return;
		}
		if (msrc == (TMINDIR|TWR)) {
			if (rsrc==BC || rsrc==DE) {
				outab(0x0A|(rsrc<<4));	/* ld a,(r16) */
				return;
			}
		}
	}
	if (src.a_type == (TBR|A)) {
		if (mdst == TSR) {
			if (rdst == I)
				outaw(0x47ED);		/* ld i,a */
			else
				outaw(0x4FED);		/* ld r,a */
			return;
		}
		if (mdst == (TMINDIR|TUSER)) {
			outab(0x32);			/* ld (addr),a */
			outraw(&dst);
			return;
		}
		if (mdst == (TMINDIR|TWR)) {
			if (rdst==BC || rdst==DE) {
				outab(0x02|(rdst<<4));	/* ld (r16),a */
				return;
			}
		}
	}
	if (dst.a_type==(TWR|SP) && msrc==TWR) {
		if (rsrc == HL) {
			outab(0xF9);			/* ld sp,hl */
			return;
		}
	}
	if (msrc == TUSER) {
		if (mdst == TBR) {
			outab(0x06|(rdst<<3));		/* ld r8,#n */
			if (indexap != NULL)
				outrab(indexap);
			outrab(&src);
			return;
		}
		if (mdst == TWR) {
			outab(0x01|(rdst<<4));		/* ld r16,#n */
			outraw(&src);
			return;
		}
	}
	if (mdst==TWR && msrc==(TMINDIR|TUSER)) {
		if (rdst == HL)
			outab(0x2A);			/* ld hl,(xxxx) */
		else
			outaw(0x4BED|(rdst<<12));	/* ld rp,(ppqq) */
		outraw(&src);
		return;
	}
	if (mdst==(TMINDIR|TUSER) && msrc==TWR) {
		if (rsrc == HL)
			outab(0x22);			/* ld (xxxx),hl */
		else
			outaw(0x43ED|(rsrc<<12));	/* ld (ppqq),rp */
		outraw(&dst);
		return;
	}
	if (mdst==TBR && msrc==TBR && (rdst!=M || rsrc!=M)) {
		outab(0x40|(rdst<<3)|rsrc);
		if (indexap != NULL)
			outrab(indexap);
		return;
	}
	aerr(INVALID_REG);
}

/*
 * Read in addresses for "ld"
 * instructions. Split off the mode
 * and the register name. Adjust the register
 * name to correctly deal with the index registers
 * and for indexed addressing modes. Return the address
 * pointer "ap" if indexing is required, otherwise just
 * pass the "iap" through.
 */
static ADDR *getldaddr(ADDR *ap, int *modep, int *regp, ADDR *iap)
{
	int mode;
	int reg;

	getaddr(ap);
	mode = ap->a_type&TMMODE;
	reg  = ap->a_type&TMREG;
	switch (ap->a_type) {
	case TBR|IX:
		outab(0xDD);
		reg = M;
		iap = ap;
		break;

	case TBR|IY:
		outab(0xFD);
		reg = M;
		iap = ap;
		break;

	case TWR|IX:
		outab(0xDD);
		reg = HL;
		break;

	case TWR|IY:
		outab(0xFD);
		reg = HL;
		break;

	case TWR|AF:
	case TWR|AFPRIME:
		aerr(INVALID_REG);
		reg = HL;
	}
	*modep = mode;
	*regp  = reg;
	return (iap);
}

/*
 * Output an opcode, surrounded
 * by the index prefix bytes and the
 * index displacement byte. Look at
 * the address mode to see if the bytes
 * are needed.
 */
static void outop(int op, ADDR *ap)
{
	int needisp;

	needisp = 0;
	if (ap->a_type == (TBR|IX)) {
		outab(0xDD);
		needisp = 1;
	} else if (ap->a_type == (TBR|IY)) {
		outab(0xFD);
		needisp = 1;
	}
	if ((op&0xFF00) != 0) {
		outab(op>>8);
		if (needisp != 0) {
			outrab(ap);
			needisp = 0;
		}
	}
	outab(op);
	if (needisp != 0)
		outrab(ap);
}

/*
 * Try to interpret an "ADDR"
 * as a condition code name. Return
 * the condition, or "-1" if it cannot
 * be interpreted as a condition. The
 * "c" condition is a pain.
 */
static int ccfetch(ADDR *ap)
{
	if (ap->a_type == (TBR|C))
		return (CC);
	if ((ap->a_type&TMMODE) == TCC)
		return (ap->a_type&TMREG);
	return (-1);
}
