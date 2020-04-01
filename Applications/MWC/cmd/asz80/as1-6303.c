/*
 * 6303 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"


/*
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
	/* FIXME: if we meet 35 how do we know whether its ZP or
	   not. I guess we just assume as direct is shorter anyway */
	/* If it's not a constant then don't play with it, but rely upon
	   the segment of the symbol */
	if (ap->a_segment != ABSOLUTE || ap->a_sym)
		return;
	if (ap->a_value > 255)
		return;
	/* We will need to do something saner ifwe add 68HC11 */
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
		constify(ap);
		istuser(ap);
		ap->a_type |= TIMMED;
		return;
	}

	unget(c);	
	expr1(ap, LOPRI, 1);
	c = getnb();
	
	/* foo,[x|y] */
	if (c == ',') {
		c = getnb();
		if (tolower(c) == 'x') {
			if (ap->a_value < 0 || ap->a_value > 255)
				aerr(INDX_RANGE);
			istuser(ap);
			constify(ap);
			ap->a_type |= TINDEX;
			return;
		}
		unget(c);
	}
	unget(c);
	constant_to_zp(ap);
	if (ap->a_segment == ZP)
		ap->a_type = TDIRECT|TUSER|TMINDIR;
	else
		ap->a_type = TUSER|TMINDIR;
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
		segment = ABSOLUTE;
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

	case TIMPL6303:
		/* Until we implement CPU type checks */
	case TIMPL:
		outab(opcode);
		break;

	case TXE:
		getaddr(&a1);
		if ((a1.a_type & TMADDR) != TINDEX)
			opcode += 0x10;
		outab(opcode);
		break;

	case TDIXE:
	case TDXE:
		getaddr(&a1);
		switch(a1.a_type & TMADDR) {
		case TIMMED:
			if ((sp->s_type&TMMODE) == TDXE)
				aerr(ADDR_REQUIRED);
			outab(opcode);
			istuser(&a1);
			outrab(&a1);
			break;
		case TDIRECT:
			outab(opcode + 0x10);
			constify(&a1);
			istuser(&a1);
			outrab(&a1);
			break;
		case TINDEX:
			outab(opcode + 0x20);
			outab(a1.a_value);
			break;
		default:
			/* An address */
			constify(&a1);
			istuser(&a1);
			outraw(&a1);
			break;
		}
		break;

	case T16DIXE:
	case T16DXE:
		getaddr(&a1);
		switch(a1.a_type & TMADDR) {
		case TIMMED:
			if ((sp->s_type&TMMODE) == T16DXE)
				aerr(ADDR_REQUIRED);
			outab(opcode);
			istuser(&a1);
			outraw(&a1);
			break;
		case TDIRECT:
			outab(opcode + 0x10);
			outab(a1.a_value);
			break;
		case TINDEX:
			outab(opcode + 0x20);
			outab(a1.a_value);
			break;
		default:
			/* An address */
			constify(&a1);
			istuser(&a1);
			outraw(&a1);
			break;
		}
		break;

	case TREL8:
		getaddr(&a1);
		disp = a1.a_value-dot[segment]-2;
		if (disp<-128 || disp>127 || a1.a_segment != segment)
			aerr(BRA_RANGE);
		outab(opcode);
		outab(disp);
		break;

		outab(opcode);
		outraw(&a1);
		break;

	case TIDX6303:	/* 6303 oddity, immediate followed by direct or indexed */
		getaddr(&a1);
		if ((a1.a_type & TMADDR) != TIMMED)
			qerr(SYNTAX_ERROR);

		if (a1.a_value < 0 || a1.a_value > 255)
			aerr(CONSTANT_RANGE);

		c = getnb();
		if (c != ',')
			qerr(SYNTAX_ERROR);

		getaddr(&a2);
		switch(a2.a_type & TMADDR) {
			case TDIRECT:
				opcode += 0x10;
				break;
			case TINDEX:
				break;
			default:
				qerr(SYNTAX_ERROR);
				break;
		}
		/* Now encode the instruction */
		outab(opcode);
		outrab(&a1);
		outrab(&a2);
		break;

	case TIDXB6303:	/* The above - rebranded as bit operations */
		getaddr(&a1);
		switch (a1.a_type & TMADDR) {
			case TINDEX:
			case TIMMED:
				qerr(SYNTAX_ERROR);
		}
		constify(&a1);
		/* Bit number */
		if (a1.a_value < 0 || a1.a_value > 7)
			aerr(CONSTANT_RANGE);
		c = getnb();
		if (c != ',')
			qerr(SYNTAX_ERROR);

		getaddr(&a2);
		switch(a2.a_type & TMADDR) {
			case TDIRECT:
				opcode += 0x10;
				break;
			case TINDEX:
				break;
			default:
				aerr(SYNTAX_ERROR);
				break;
		}
		/* Mangle the bit value into the right format */
		a1.a_value = 1 << a1.a_value;
		/* For AIM aka BCLR we need to invert it */
		a1.a_value ^= opcode >> 8;
		/* Now encode the instruction */
		outab(opcode & 0xFF);
		outrab(&a1);
		outrab(&a2);
		break;
			

	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
