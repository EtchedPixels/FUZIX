/*
 * 6502 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"

static unsigned idx_size;
static unsigned acc_size;
static unsigned cputype = CPU_6502;
static uint8_t reltab[1024];
static unsigned int nextrel;

int passbegin(int pass)
{
	cputype = CPU_6502;
	segment = 1;
	idx_size = 1;
	acc_size = 1;
	if (pass == 3)
		nextrel = 0;
	return 1;
}

static void setnextrel(int flag)
{
	if (nextrel == 8 * sizeof(reltab))
		aerr(TOO_MANY_BRA);
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

static void require_cpu(unsigned x)
{
	if (cputype < x)
		aerr(BADCPU);
}

static int requirexy(ADDR *ap)
{
	int r = (ap->a_type & TMREG);
	if ((ap->a_type & TMMODE) != TBR || (r != X  && r != Y))
		aerr(INVALID_REG);
	return r;
}

/*
 * Deal with the syntactic mess 6502 assembler has
 *
 * In some cases (JSR JMP and definitions - eg .word)
 * $ABCD means a constant everywhere else that is #ABCD
 */

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

static void make_zp(ADDR *ap)
{
	/* TODO: deal with non zero DP via some kind of option. Also
	   we should check syms we know are not ZP and error them. Right now
	   this will catch it anyway but for the wrong reasons */
	if (ap->a_value > 255) {
		qerr(RANGE);
		return;
	}
	ap->a_segment = ZP;
	return;
}

/* Get an expression that is implicitly zero page */
static void expr1_zp(ADDR *ap)
{
	unsigned c = getnb();
	if (c != '@')
		unget(c);
	expr1(ap, LOPRI, 0);
	make_zp(ap);
}
	
/* Get an expression that might be zero page */
static void expr1_maybe_zp(ADDR *ap)
{
	unsigned c = getnb();
	if (c != '@')
		unget(c);
	expr1(ap, LOPRI, 0);
	if (c == '@')
		make_zp(ap);
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
	ADDR tmp;

	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;
	
	c = getnb();

	/* #foo */	
	if (c == '#') {
		c = getnb();
		if (c == '<')
			ap->a_flags |= A_LOW;
		else if (c == '>')
			ap->a_flags |= A_HIGH;
		else
			unget(c);
		expr1_maybe_zp(ap);
		istuser(ap);
		return;
	}
	/* (foo),y and (foo,x) */
	/* For 65C02 also (foo) */
	if (c == '(') {
		expr1_zp(ap);
		c = getnb();
		/* (xxx) -- check for ,y */
		if (c == ')') {
			c = getnb();
			if (c  == ',') {
				expr1(&tmp, LOPRI, 0);
				if ((reg = requirexy(&tmp)) == 0)
					aerr(INVALID_REG);
				if (reg == Y)
					ap->a_type |= TZPY_IND;
				else
					aerr(INVALID_REG);
				return;
			}
			unget(c);
			ap->a_type = TZP_IND;
			return;
		}
		/* (xxx,) check for X */
		if (c == ',') {
			expr1(&tmp, LOPRI, 0);
			if (getnb() != ')')
				aerr(BRACKET_EXPECTED);
			if ((reg = requirexy(&tmp)) == 0)
				aerr(INVALID_REG);
			if (reg == Y)
				aerr(INVALID_REG);
			else
				ap->a_type |= TZPX_IND;
			return;
		}
		aerr(BRACKET_EXPECTED);
	}
	unget(c);

	expr1_maybe_zp(ap);
	
	c = getnb();
	
	/* foo,[x|y] */
	if (c == ',') {
		/* FIXME: if we meet 35,x how do we know whether its ZP or
		   not. This matters on non 6502 variants. We can't just guess
		   either if the value comes from a symbol because we might
		   learn the value on phase 1 and then we'd change size and
		   get a phase error.
		   
		   Worse yet there are cases where abs,y is valid but not zp
		   (class 1 instructions), and where zp,y is but not abs,y (stx) */

		/* FIXME: in 16bit mode these are not ZP */
		expr1(&tmp, LOPRI, 0);
		if ((reg = requirexy(&tmp)) == 0)
			aerr(INVALID_REG);
		if (reg == X) {
			if (ap->a_segment == ZP)
				ap->a_type |= TZPX;
			else
				ap->a_type |= TABSX;
		} else {
			if (ap->a_segment == ZP)
				ap->a_type |= TZPY;
			else
				ap->a_type |= TABSY;
		}
		return;
	}
	unget(c);
	if ((ap->a_type & TMMODE) == TBR && (ap->a_type & TMREG) == A)
		ap->a_type = TACCUM;
	else { /* absolute or zp */
		if (ap->a_segment == ZP)
			ap->a_type = TZP;
		else
			ap->a_type = TUSER|TMINDIR;
	}
}

/* Handle the corner case of labels in direct page being used as relative
   branches from the overlapping 'absolute' space. Assumes DP = 0 */
static int segment_incompatible(ADDR *ap)
{
	if (ap->a_segment == segment)
		return 0;
	if (ap->a_segment == 4 && segment == 0 && ap->a_value < 256)
		return 0;
	return 1;
}

uint8_t class2_mask(uint8_t opcode, uint16_t type, uint8_t mode)
{
	uint8_t r;

	switch(type & TMADDR) {
		case 0:
			if (type & TMINDIR)
				r = 3;
			else
				r = 0;
			break;
		case TZP:
			r = 1;
			break;
		case TACCUM:
			if (opcode & 0x80)
				qerr(BADMODE);
			r = 2;
			break;
		case TZPX:
			if (mode == 1)
				qerr(BADMODE);
			r = 5;
			break;
		case TZPY:
			if (mode == 0)
				qerr(BADMODE);
			r = 5;
			break;
		case TABSX:
			if (mode == 0)
				r = 7;
			else
				qerr(BADMODE);
			break;
		case TABSY:
			if (mode == 1)
				r = 7;
			else
				qerr(BADMODE);
			break;
		default:
			aerr(BADMODE);
			break;
	}
	return r;
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
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;
	ADDR a2;
	unsigned size;

loop:
	if ((c=getnb())=='\n' || c==';')
		return;
	if (isalpha(c) == 0 && c != '_' && c != '.')
		qerr(UNEXPECTED_CHR);
	getid(id, c);
	if ((c=getnb()) == ':') {
		sp = lookup(id, uhash, 1);
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
		constify(&a1);
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
		constify(&a1);
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
			constify(&a1);
			istuser(&a1);
			outrab(&a1);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFW:
		do {
			getaddr(&a1);
			constify(&a1);
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
		constify(&a1);
		istuser(&a1);
		/* Write out the bytes. The BSS will deal with the rest */
		for (value = 0 ; value < a1.a_value; value++)
			outab(0);
		break;

	case TCPU:
		cputype = opcode;
		break;
	case TI:
		idx_size = opcode;
		break;
	case TA:
		acc_size = opcode;
		break;

	case TREL8:	/* Branch */
		/* Algorithm:
			Pass 0: generate worst case code. We then know things
				that can safely be turned short because more
				shortening will only reduce gap
			Pass 1: generate code case based upon pass 0 but now
				using short branch conditionals
			Pass 2: repeat this because pass 1 causes a lot of
				collapses. Pin down the choices we made.
			Pass 3: generate code. We don't "fix" any further
				possible shortenings because we need the
				addresses in pass 3 to exactly match pass 2
		*/
		getaddr(&a1);
		disp = a1.a_value - dot[segment] - 2;
		if (pass == 3)
			c = getnextrel();
		else {
			c = 0;
			if (pass == 0 || segment_incompatible(&a1) || disp<-128 || disp>127 )
				c = 1;
			if (pass == 2)
				setnextrel(c);
		}
		if (c) {
			outab(opcode ^ 0x20);	/* reverse branch */
			outab(3);	/* Over the jump */
			outab(0x4C);	/* JMP */
			outraw(&a1);
		} else {
			outab(opcode);
			/* Should never happen */
			if (disp < -128 || disp > 127)
				aerr(BRA_RANGE);
			outab(disp);
		}
		break;
	case TREL8C:
		/* Similar but without condition so we can just swap for
		   a jump or brl */
		require_cpu(CPU_65C02);
		getaddr(&a1);
		disp = a1.a_value - dot[segment] - 2;
		if (pass == 3)
			c = getnextrel();
		else {
			c = 0;
			if (pass == 0 || segment_incompatible(&a1) || disp<-128 || disp>127 )
				c = 1;
			if (pass == 2)
				setnextrel(c);
		}
		if (c) {
			outab(0x4C);	/* JMP */
			outraw(&a1);
		} else {
			outab(opcode);
			/* Should never happen */
			if (disp < -128 || disp > 127)
				aerr(BRA_RANGE);
			outab(disp);
		}
		break;
	case TREL16:
		require_cpu(CPU_65C816);
		getaddr(&a1);
		if (a1.a_segment != segment)
			aerr(BRA_RANGE);
		/* Check this is right TODO */
		disp = a1.a_value-dot[segment]-3;
		outab(opcode);
		outab(disp);
		outab(disp >> 8);
		break;
	case TJMP:
		/* jmp has the weird unique (xxxx) indirect */
		c = getnb();
		if (c == '(') {
			getaddr(&a1);
			if (getnb() != ')')
				qerr(BRACKET_EXPECTED);
			if ((a1.a_type & TMADDR) == TABSX) {
				require_cpu(CPU_65C02);
				outab(0x7C);
				outraw(&a1);
				break;
			}
			if (a1.a_type != TZP && a1.a_type != (TUSER|TMINDIR))
				aerr(BADMODE);
			outab(0x60);
			outraw(&a1);
			break;
		}
		unget(c);
		getaddr(&a1);
		if (a1.a_type != (TUSER|TMINDIR) && a1.a_type != TZP)
			aerr(BADMODE);
		outab(opcode);
		outraw(&a1);
		break;
	case TJSR:
		/* JSR has absolute indexed indirect as a special case */
		c = getnb();
		if (c == '(') {
			getaddr(&a1);
			if (getnb() != ')')
				qerr(BRACKET_EXPECTED);
			if ((a1.a_type & TMADDR) != TABSX)
				aerr(BADMODE);
			outab(0xFC);
			outraw(&a1);
			break;
		}
		unget(c);
		getaddr(&a1);
		if (a1.a_type != (TUSER|TMINDIR) && a1.a_type != TZP)
			aerr(BADMODE);
		outab(opcode);
		outraw(&a1);
		break;

	case TBRK:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outab(opcode);
		outrab(&a1);
		break;

	case TIMPL16:
		require_cpu(CPU_65C816);
		outab(opcode);
		break;
	case TIMPLC:
		require_cpu(CPU_65C02);
		/* Fall through */
	case TIMPL:
		outab(opcode);
		break;

	case TCLASS0:
		/* 0x80 ldy 0xA0 sty 0xC0 cpy 0xE0 cpx */
		getaddr(&a1);
		reg = class2_mask(opcode, a1.a_type, 0);
		/*  No accumulator */
		if (reg == 2 || reg == 4 || reg == 6)
			aerr(BADMODE);
		/* No store immediate */
		if (reg == 0 && opcode == 0x80)
			aerr(BADMODE);
		outab(opcode | reg << 2);
		if (reg == 3 || reg == 7 || (reg == 0 && idx_size == 2))
			outraw(&a1);
		else
			outrab(&a1);
		break;

	case TCLASS0X:	/* BIT is weird */
		getaddr(&a1);
		switch(a1.a_type & TMADDR) {
			case 0:
				if (a1.a_type & TMINDIR) {
					outab(0x2C);
					outraw(&a1);
				} else {
					/* Immediate */
					require_cpu(CPU_65C02);
					outab(0x89);
					if (acc_size == 2)
						outraw(&a1);
					else
						outrab(&a1);
				}
				break;
			case TZP:
				outab(0x24);
				outrab(&a1);
				break;
			case TABSX:
				outab(0x3C);
				outraw(&a1);
				break;
			case TZPX:
				outab(0x34);
				outrab(&a1);
				break;
			default:
				qerr(BADMODE);
		}
		break;

	case TCLASS1:
		getaddr(&a1);
		size = 1;
		switch(a1.a_type & TMADDR) {
			case TZPX_IND:
				/* (zp,x) */
				reg = 0;
				break;
			case TZP: /* zp */
				reg = 0x04;
				break;
			case 0:	/* #imm or abs */
				if (a1.a_type & TMINDIR) {
					/* Absolute */
					reg = 0x0C;
					size = 2;
				} else {
					/* Immediate */
					reg = 0x08;
					size = acc_size;
				}
				break;
			case TZPY_IND:	/* (zp),y */
				reg = 0x10;
				break;
			case TZPX:	/* zp, x */
				reg = 0x14;
				break;
			/* FIXME: this is only safe on the classic 6502 */
			case TZPY:	/* zp, y */
				/* Fall through and use ABS,Y */
			case TABSY:	/*  abs, y */
				reg = 0x18;
				size = 2;
				break;
			case TABSX:	/* abs, x */
				reg = 0x1C;
				size = 2;
				break;
			case TZP_IND:	/* (dp) */
				require_cpu(CPU_65C02);
				reg = 0x11;	/* Slightly odd case */
				break;
			case TZP_INDL:	/* [dp] */
				require_cpu(CPU_65C816);
				reg = 0x06;
				break;
			case TALX_IND:	/* long,X */
				require_cpu(CPU_65C816);
				reg = 0x1E;
				size = 3;
				break;
			case TZPYL_IND:	/* [dp],Y */
				require_cpu(CPU_65C816);
				reg = 0x16;
				break;
			case TSR:	/* sr,S */
				require_cpu(CPU_65C816);
				reg = 0x02;
				break;
			case TSRY_IND: 	/* (sr,S),Y */
				require_cpu(CPU_65C816);
				reg = 0x12;
				break;
			case TABSL:	/* long absolute */
				require_cpu(CPU_65C816);
				size = 3;
				reg = 0x0E;
				break;
			default:
				aerr(BADMODE);
				break;
		}
		opcode &= 0xFF;
		opcode |= reg;
		if (opcode == 0x89)	/* sta immediate */
			qerr(BADMODE);
		outab(opcode);
		/* TODO: size == 3 cases */
		if (size == 3)
			aerr(BADMODE);	/* FIXME */
		if (size == 2)
			outraw(&a1);
		else if (size == 1)
			outrab(&a1);
		break;
	case TCLASS2:
		getaddr(&a1);
		reg = class2_mask(opcode, a1.a_type, 0);
		if (reg == 0)
			aerr(BADMODE);
		/* No store immediate */
		if (reg == 3 && opcode == 0xA0)
			aerr(BADMODE);
		outab(opcode | (reg << 2));
		if (reg == 0 && idx_size == 2)
			outraw(&a1);
		else if (reg < 2)
			outrab(&a1);
		else if (reg == 3 || reg == 7)
			outraw(&a1);
		break;
	case TCLASS2A:
		/* Like class 2 but has unrelated accumulator encoding */
		getaddr(&a1);
		if ((a1.a_type & TMADDR) == TACCUM) {
			require_cpu(CPU_65C02);
			outab(opcode >> 8);
			break;
		}
		reg = class2_mask(opcode, a1.a_type, 0);
		/* No immediates */
		if (reg == 0)
			aerr(BADMODE);
		outab((opcode  & 0xFF)| (reg << 2));
		if (reg < 2)
			outrab(&a1);
		else if (reg == 3 || reg == 7)
			outraw(&a1);
		break;
	case TCLASS2Y:
		/* Like class 2 but has ,y not ,x forms */
		getaddr(&a1);
		reg = class2_mask(opcode, a1.a_type, 1);
		outab(opcode | (reg << 2));
		/* No store immediate */
		if (reg == 0 && opcode == 0x82)
			aerr(BADMODE);
		if (reg == 0 && idx_size == 2)
			outraw(&a1);
		else if (reg < 2)
			outrab(&a1);
		else if (reg == 3 || reg == 7)
			outraw(&a1);
		break;
	case TIMM16:
		require_cpu(CPU_65C816);
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outab(opcode);
		outraw(&a1);
		break;
	case TSTZ:
		/* STZ has odd encodings but support abs, dp, ai,x dp,x */
		require_cpu(CPU_65C02);
		getaddr(&a1);
		switch(a1.a_type & TMADDR) {
		case TABSX:
			outab(0x9E);
			outraw(&a1);
			break;
		case TZPX:
			outab(0x74);
			outrab(&a1);
			break;
		case TZP:
			outab(0x64);
			outrab(&a1);
			break;
		case 0:
			if (!(a1.a_type & TMINDIR))
				qerr(BADMODE);
			outab(0x9C);
			outraw(&a1);
			break;
		}
		break;
	case TABDP:
		require_cpu(CPU_65C816);
		getaddr(&a1);
		switch(a1.a_type & TMADDR) {
		case 0:
			if (!(a1.a_type & TMINDIR))
				qerr(BADMODE);
			outab(opcode >> 8);
			outraw(&a1);
			break;
		case TZP:
			outab(opcode);
			outrab(&a1);
			break;
		default:
			qerr(BADMODE);
		}
		break;
	case TMVN:
		require_cpu(CPU_65C816);
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		comma();
		getaddr(&a2);
		constify(&a2);
		istuser(&a2);
		if (a1.a_value < 0 || a2.a_value < 0 ||
			a1.a_value > 255 || a2.a_value > 255)
			qerr(RANGE);
		outab(opcode);
		outab(a1.a_value);
		outab(a2.a_value);
		break;
	case TPEI:
		require_cpu(CPU_65C816);
		getaddr(&a1);
		if ((a1.a_type & TMADDR) != TZP_IND)
			qerr(BADMODE);
		outab(opcode);
		outrab(&a1);
		break;
	case TREP:
		require_cpu(CPU_65C816);
		getaddr(&a1);
		if ((a1.a_type & (TMADDR|TMINDIR)) != 0)
			qerr(BADMODE);
		if (a1.a_value < 0 || a1.a_value > 255)
			qerr(RANGE);
		outab(opcode);
		outab(a1.a_value);
		break;
	/* TODO */
	case TJML:
		require_cpu(CPU_65C816);
		qerr(BADMODE);
	case TLONG:
		require_cpu(CPU_65C816);
		qerr(BADMODE);
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
