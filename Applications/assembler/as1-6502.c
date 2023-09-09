/*
 * 6502 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"


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

static void constant_to_zp(ADDR *ap)
{
	/* FIXME: if we meet 35,x how do we know whether its ZP or
	   not. This matters on non 6502 variants. We can't just guess
	   either if the value comes from a symbol because we might
	   learn the value on phase 1 and then we'd change size and
	   get a phase error.

	   Worse yet there are cases where abs,y is valid but not zp
	   (class 1 instructions), and where zp,y is but not abs,y (stx) */

	/* If it's not a constant then don't play with it, but rely upon
	   the segment of the symbol */
	if (ap->a_segment != ABSOLUTE || ap->a_sym)
		return;
	if (ap->a_value > 255)
		return;
	/* We will need to do something saner on 65C816 and some of the
	   other weird variants */
	ap->a_segment = ZP;
	return;
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
		expr1(ap, LOPRI, 0);
		istuser(ap);
		return;
	}
	/* (foo),[x|y] */
	if (c == '(') {
		expr1(ap, LOPRI, 0);
		if (getnb() != ')')
			qerr(BRACKET_EXPECTED);
		/* For 65C02 we need to add the commaless mode */
		comma();
		expr1(&tmp, LOPRI, 0);
		if ((reg = requirexy(&tmp)) == 0)
			aerr(INVALID_REG);
		if (reg == X)
			ap->a_type |= TZPX_IND;
		else
			ap->a_type |= TZPY_IND;
		return;
	}
	unget(c);
	
	expr1(ap, LOPRI, 1);
	c = getnb();
	
	/* foo,[x|y] */
	if (c == ',') {
		expr1(&tmp, LOPRI, 0);
		if ((reg = requirexy(&tmp)) == 0)
			aerr(INVALID_REG);
		/* FIXME: if we meet 35,x how do we know whether its ZP or
		   not. This matters on non 6502 variants. We can't just guess
		   either if the value comes from a symbol because we might
		   learn the value on phase 1 and then we'd change size and
		   get a phase error.
		   
		   Worse yet there are cases where abs,y is valid but not zp
		   (class 1 instructions), and where zp,y is but not abs,y (stx) */

		constant_to_zp(ap);
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
		constant_to_zp(ap);
		if (ap->a_segment == ZP)
			ap->a_type = TZP_IND;
		else
			ap->a_type = TUSER|TMINDIR;
	}
}

uint8_t class2_mask(uint8_t opcode, uint16_t type, uint8_t mode)
{
	uint8_t r;

	switch(type & TMADDR) {
		case 0:
			if (type & TMINDIR)
				r = 3;
			else {
				r = 0;
				if (opcode != 0xA2)
					qerr(BADMODE);
			}
			break;
		case TZP_IND:
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
			if (opcode != 0xB2)
				r = 7;
			else
				qerr(BADMODE);
			break;
		case TABSY:
			if (opcode == 0xB2)
				r = 7;
			else
				qerr(BADMODE);
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
	int srcreg;
	int cc;
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;
	ADDR a2;
	int user;

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

	case TREL8:
		getaddr(&a1);
		disp = a1.a_value-dot[segment]-2;
		if (disp<-128 || disp>127 || a1.a_segment != segment)
			aerr(BRA_RANGE);
		outab(opcode);
		outab(disp);
		break;

	case TJMP:
		/* jmp has the weird unique (xxxx) indirect */
		c = getnb();
		if (c == '(') {
			getaddr(&a1);
			if (getnb() != ')')
				qerr(BRACKET_EXPECTED);
			outab(0x60);
			outraw(&a1);
			break;
		}
		/* If not fall through */
	case TJSR:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
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

	case TIMPL:
		outab(opcode);
		break;

	case TCLASS0:
		getaddr(&a1);
		reg = class2_mask(opcode, a1.a_type, 0);
		if (reg == 2 || reg == 4 || reg == 6)
			aerr(BADMODE);
		/* Only a subset of these exist */
		if (reg == 7 && opcode != 0xB0)
			aerr(BADMODE);
		else if (reg > 3 && opcode != 0x90)
			aerr(BADMODE);
		else if (reg == 7 && opcode < 0xA0)
			aerr(BADMODE);
		outab(opcode | reg << 2);
		if (reg == 3 || reg == 7)
			outraw(&a1);
		else
			outrab(&a1);
		break;

	case TCLASS1:
		getaddr(&a1);
		switch(a1.a_type & TMADDR) {
			case TZPX_IND:
				reg = 0;
				break;
			case TZP_IND:
				reg = 1;
				break;
			case 0:
				reg = 2;
				if (a1.a_type & TMINDIR)
					reg = 3;
				break;
			case TZPY_IND:
				reg = 4;
				break;
			case TZPX:
				reg = 5;
				break;
			/* FIXME: this is only safe on the classic 6502 */
			case TZPY:
				/* Fall through and use ABS,Y */
			case TABSY:
				reg = 6;
				break;
			case TABSX:
				reg = 7;
				break;
			default:
				aerr(BADMODE);
				break;
		}
		opcode |= (reg << 2);
		if (opcode == 0x89)	/* sta immediate */
			qerr(BADMODE);
		outab(opcode);
		if (reg == 3 || reg > 5)
			outraw(&a1);
		else if (reg == 1 || reg == 2)
			outrab(&a1);
		break;
	case TCLASS2:
		getaddr(&a1);
		reg = class2_mask(opcode, a1.a_type, 0);
		outab(opcode | (reg << 2));
		if (reg < 2)
			outrab(&a1);
		else if (reg == 3 || reg == 7)
			outraw(&a1);
		break;
	case TCLASS2Y:
		getaddr(&a1);
		reg = class2_mask(opcode, a1.a_type, 1);
		outab(opcode | (reg << 2));
		if (reg > 2)
			outrab(&a1);
		else if (reg == 3 || reg == 7)
			outraw(&a1);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
