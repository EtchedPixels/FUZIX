/*
 * Z8 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 *
 * Differences from the 'standard' Z8 stuff
 *
 * - 0x is used for hex not %XX
 * - %(base)foo is not supported
 * - %L etc for line feed and the like are not supported
 * - None of the PLZ stuff
 * - ; is used for comments not '!' or '//'
 * - manufactured instructions (RES etc) are not yet supported (use the
 *   underlying and/or immediate)
 * - no built in register alias names - .EQU in ABS should do fine ?
 */
#include	"as.h"

int passbegin(int pas)
{
	segment = 1;
	/* No branch adjustment passes */
	if (pass == 1 || pass == 2)
		return 0;
	return 1;
}

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

/*
 * Read in an address
 * descriptor, and fill in
 * the supplied "ADDR" structure
 * with the mode and value.
 * Exits directly to "qerr" if
 * there is no address field or
 * if the syntax is bad.
 *
 * Z8 syntax wants %xx for hex and HI xx or LO xx not < >. Look at that
 * later.
 *
 * We recognize the following
 * Rn - register short form
 * RRn - register pair short form
 * @Rn - register, indirect short form
 * @RRn - register pair, indirect short form
 * n - register
 * @n - register indirect
 * #n - 8bit constant
 */
void getaddr_r(ADDR *ap)
{
	int indirect = 0;
	int pair = 0;
	int c;

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
		constify(ap);
		ap->a_type |= TIMMED;
		return;
	}
	
	if (c == '@') {
		indirect = 1;
		c = getnb();
	}
	/* Register short forms */
	if (c == 'R' || c == 'r') {
		c = getnb();
		if (c == 'R' || c == 'r')
			pair = 1;
		else
			unget(c);
			
		expr1(ap, LOPRI, 0);
		istuser(ap);
		constify(ap);
		if (ap->a_value < 0 || ap->a_value > 15)
			aerr(RSHORT_RANGE);
		if (pair == 0) {
			if (indirect)
				ap->a_type |= TSIND;
			else
				ap->a_type |= TRS;
		} else {
			if (indirect)
				ap->a_type |= TRRIND;
			else {
				if (ap->a_value & 1)
					aerr(ODD_REGISTER);
				ap->a_type |= TRR;
			}
		}
		return;
	}
	unget(c);

	/* If it wasn't a short register or a constant it's a register. We
	   have no 'address' formats except for constant load and indexed */

	expr1(ap, LOPRI, 1);

	/* Must be an 8bit result */
	if (ap->a_value < -128 || ap->a_value > 255)
		qerr(CONSTANT_RANGE);
	c = getnb();
	if (c == '(') {
		ADDR tmp;
		ap->a_type = TINDEX;
		if (indirect)
			aerr(INVALID_FORM);
		getaddr(&tmp);
		if ((tmp.a_type & TMADDR) != TRS)
			qerr(RSHORT_RANGE);
		c = getnb();
		if (c != ')')
			qerr(SYNTAX_ERROR);
		ap->a_value &= 0xFF;
		ap->a_value |= (tmp.a_value << 8);	/* Hackish */
	} else {
		unget(c);
		if (indirect)
			ap->a_type = TIND;
		else
			ap->a_type = TREG;
	}
}

void getaddr8(ADDR *ap)
{
	getaddr_r(ap);
	if (ap->a_value < -128 || ap->a_value > 255)
		aerr(CONSTANT_RANGE);
}
		
int getcond(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == TCC)
		return (ap->a_type & TMREG);
	return -1;
}

void getaddr(ADDR *ap)
{
	int c = getnb();
	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);
	expr1(ap, LOPRI, 0);
	/* Condition code */
	if ((ap->a_type & TMMODE) == TCC)
		return;
	istuser(ap);
	constify(ap);
	ap->a_type |= TIMMED;
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
	int cc;
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;
	ADDR a2;
	int ta1;
	int ta2;

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

	case TIMPL:
		outab(opcode);
		break;

	/* Logic and maths operations with a 4 bit operation and 4 bits of
	   addressing information */
	case TOP4BIT:
		getaddr8(&a1);
		c = getnb();
		if (c != ',')
			qerr(MISSING_DELIMITER);
		getaddr8(&a2);
		/* Not work out how to encode it */
		/* OP r,r */
		ta1 = a1.a_type & TMADDR;
		ta2 = a2.a_type & TMADDR;

		/* Short forms are  dest << 4 | soure  */
		if (ta1 == TRS && ta2 == TRS) {
			outab(opcode | 0x02);
			outab((a1.a_value << 4) | a2.a_value);
			break;
		}
		/* OP r,Ir */
		if (ta1 == TRS && ta2 == TSIND) {
			outab(opcode | 0x03);
			outab((a1.a_value << 4) | a2.a_value);
			break;
		}
		/* Look for long forms using 0xE0 for current reg set */
		if (ta1 == TRS) {
			a1.a_value |= 0xE0;
			ta1 = TREG;
		}
		if (ta1 == TSIND) {
			a1.a_value |= 0xE0;
			ta1 = TIND;
		}
		if (ta2 == TRS) {
			a2.a_value |= 0xE0;
			ta2 = TREG;
		}
		if (ta2 == TSIND) {
			a2.a_value |= 0xE0;
			ta2 = TIND;
		}
		/* Long forms  are src, dst except when the are not (sigh)*/
		/* OP R,R */
		if (ta1 == TREG && ta2 == TREG)
			outab(opcode | 0x04);
		/* OP R,@R */
		else if (ta1 == TREG && ta2 == TIND)
			outab(opcode | 0x05);
		/* OP R,IMM */
		else if (ta1 == TREG && ta2 == TIMMED)
			outab(opcode | 0x06);
		/* OP @R,IMM */
		else if (ta1 == TIND && ta2 == TIMMED)
			outab(opcode | 0x07);
		else
			qerr(INVALID_FORM);
		/* Immediate is backwards to the others */
		if (ta2 == TIMMED) {
			/* dst src */
			outab(a1.a_value);
			outrab(&a2);
		} else {
			/* src dst */
			outab(a2.a_value);
			outab(a1.a_value);
		}
		break;

	case TRRIR:
		/* RR or IR format */
		getaddr8(&a1);
		switch(a1.a_type & TMADDR) {
		case TREG:
			if (a1.a_value & 1)
				aerr(ODD_REGISTER);
			outab(opcode);
			break;
		case TIND:
			outab(opcode + 0x10);
			break;
		default:
			qerr(INVALID_FORM);
		}
		outab(a1.a_value);
		break;
	case TRIR:
		/* R or IR format */
		getaddr8(&a1);
		switch(a1.a_type & TMADDR) {
		case TREG:
			outab(opcode);
			break;
		case TIND:
			outab(opcode + 0x10);
			break;
		default:
			qerr(INVALID_FORM);
		}
		outab(a1.a_value);
		break;

	case TIMM8:
		/* 8bit immediate */
		getaddr8(&a1);
		if ((a1.a_type & TMADDR) != TIMMED)
			qerr(INVALID_FORM);
		outab(opcode);
		outab(a1.a_value);
		break;

	case TCRA:
		getaddr(&a1);
		cc = getcond(&a1);
		if (cc != -1) {
			c = getnb();
			if (c != ',')
				qerr(MISSING_DELIMITER);
			getaddr(&a1);
		} else
			cc = 0x08; /* True */
		outab(opcode + (cc << 4));
		a1.a_value -= dot[segment] + 1;
		/* Relative branches are always in segment and within our
		   generated space so don't relocate */
		outrabrel(&a1);
		break;
	case TJMP:
		getaddr(&a1);
		cc = getcond(&a1);
		if (cc != -1) {
			c = getnb();
			if (c != ',')
				qerr(MISSING_DELIMITER);
			getaddr(&a1);
		} else
			cc = 0x08; /* True */
		if ((a1.a_type & TMADDR) == TRRIND) {
			if (cc != 0x08)
				qerr(INVALID_FORM);
			/* JP @RR */
			outab(0x30);
			outab(a1.a_value);
		} else {
			outab(opcode + (cc << 4));
			/* Relocatable label */
			outraw(&a1);
		}
		break;
		
	case TIRRDA:	/* Call */
		getaddr(&a1);
		if ((a1.a_type & TMADDR) == TRRIND) {
			/* CALL @RR */
			outab(0xD4);
			outab(a1.a_value);
		} else {
			outab(0xD6);
			/* Relocatable label */
			outraw(&a1);
		}
		break;		
	case TRA:	/* DJNZ */
		getaddr8(&a1);
		if ((a1.a_type & TMADDR) != TRS)
			qerr(INVALID_FORM);
		outab(0x0A | (a1.a_value << 4));
		/* And then a relative address */
		getaddr(&a2);
		a2.a_value -= dot[segment] + 1;
		if (a2.a_value < -128 || a2.a_value > 127)
			aerr(BRA_RANGE);
		outrabrel(&a2);
		break;
	case TLDC:
		getaddr8(&a1);
		c = getnb();
		if (c != ',')
			qerr(MISSING_DELIMITER);
		getaddr8(&a2);
		ta1 = a1.a_type & TMADDR;
		ta2 = a2.a_type & TMADDR;
		if (ta1 == TRS && ta2 == TRRIND) {
			outab(opcode);
			outab((a1.a_value << 4) | a2.a_value);
			/* dst, src */
		} else if (ta1 == TRRIND && ta2 == TRS) {
			/* dst, src */
			outab(opcode + 0x10);
			outab((a2.a_value << 4) | a1.a_value);
		} else
			qerr(INVALID_FORM);
		break;
	case TLDCI:
		getaddr8(&a1);
		c = getnb();
		if (c != ',')
			qerr(MISSING_DELIMITER);
		getaddr8(&a2);
		ta1 = a1.a_type & TMADDR;
		ta2 = a2.a_type & TMADDR;
		if (ta1 == TSIND && ta2 == TRRIND) {
			/* dst, src */
			outab(opcode);
			outab((a1.a_value << 4) | a2.a_value);
			outab(a2.a_value);
		} else if (ta1 == TRRIND && ta2 == TSIND) {
			outab(opcode + 0x10);
			/* src, dst */
			outab((a1.a_value << 4) | a2.a_value);
			outab(a2.a_value);
		} else
			qerr(INVALID_FORM);
		break;
	case TLOAD:
		/* Load is its own special complicated case */
		getaddr8(&a1);
		c = getnb();
		if (c != ',')
			qerr(MISSING_DELIMITER);
		getaddr8(&a2);
		ta1 = a1.a_type & TMADDR;
		ta2 = a2.a_type & TMADDR;
		/* Now encode the load by type */
		if (ta1 == TRS && ta2 == TIMMED) {
			outab(0x0C | (a1.a_value << 4));
			outrab(&a2);
			break;
		}
		/* As with the logic/maths ops the encoding order isn't
		   entirely sane */
		if (ta1 == TRS && ta2 == TREG) {
			/* dst|mode , src */
			outab(0x08 | (a1.a_value << 4));
			outab(a2.a_value);
			break;
		}
		if (ta1 == TREG && ta2 == TRS) {
			/* src|mode , dst */
			outab(0x09 | (a2.a_value << 4));
			outab(a1.a_value);
			break;
		}
		if (ta1 == TRS && ta2 == TSIND) {
			/* mode | 3 , dst | src */
			outab(0xE3);
			outab((a1.a_value << 4) | (a2.a_value));
			break;
		}
		if (ta1 == TSIND && ta2 == TRS) {
			/* mode | 3 , dst | src */
			outab(0xF3);
			outab((a1.a_value << 4) | (a2.a_value));
			break;
		}
		if (ta1 == TRS && ta2 == TINDEX) {
			/* op , dst | x, offset */
			outab(0xC7);
			outab((a1.a_value << 4) | (a2.a_value >> 8));
			a2.a_value &= 0xFF;
			outrab(&a2);
			break;
		}
		if (ta1 == TINDEX && ta2 == TRS) {
			/* op , src | x, offset */
			outab(0xD7);
			outab((a1.a_value >>  8) | (a2.a_value << 4));
			a1.a_value &= 0xFF;
			outrab(&a1);
			break;
		}
		/* If we get here in short form we need to try the long
		   form alias */
		if (ta1 == TRS) {
			ta1 = TREG;
			a1.a_value |= 0xE0;
		}
		if (ta2 == TRS) {
			ta2 = TREG;
			a2.a_value |= 0xE0;
		}
		if (ta1 == TSIND) {
			ta1 = TINDEX;
			a1.a_value |= 0xE0;
		}
		if (ta2 == TSIND) {
			ta2 = TINDEX;
			a2.a_value |= 0xE0;
		}
		if (ta1 == TREG && ta2 == TREG) {
			/* op, src, dst */
			outab(0xE4);
			outab(a2.a_value);
			outab(a1.a_value);
			break;
		}
		if (ta1 == TREG && ta2 == TIND) {
			/* op, src, dst */
			outab(0xE5);
			outab(a2.a_value);
			outab(a1.a_value);
			break;
		}
		if (ta1 == TREG && ta2 == TIMMED) {
			/* op, dst, src */
			outab(0xE6);
			outab(a1.a_value);
			outrab(&a2);
			break;
		}
		if (ta1 == TIND && ta2 == TIMMED) {
			/* op, dst, src */
			outab(0xE7);
			outab(a1.a_value);
			outrab(&a2);
			break;
		}
		if (ta1 == TIND && ta2 == TREG) {
			/* op, src, dst */
			outab(0xF5);
			outab(a2.a_value);
			outab(a1.a_value);
			break;
		}
		qerr(INVALID_FORM);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
