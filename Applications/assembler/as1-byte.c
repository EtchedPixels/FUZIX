/*
 * Bytecodeassembler.
 * Assemble one line of input.
 * Knows all the dirt.
 *
 * First cut. Somewhat different to the default assembler right now
 * as we don't blindly default to hex and we need to sort out per arch
 * comment operators for us, Z8 and 8080
 *
 * The other limit is that you can't put short branches into non absolute
 * space because we have no idea where it would link and they are not relative
 * but rewrite the low PC byte.
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
	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;
	
	expr1(ap, LOPRI, 1);
	ap->a_type = TUSER|TMINDIR;
}

/*
 *	We don't have short/long jumps to fix up by hand so we don't
 *	want to do pass 1 and 2 just 0 and 3.
 */
int passbegin(int pass)
{
	if (pass == 1 || pass == 2)
		return 0;
	segment = 1;			/* Default to code */
	return 1;
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
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;

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
	case TBYTE:
		outab(opcode);
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		/* Turn into an unsigned byte for the encoder */
		if (a1.a_segment == ABSOLUTE && !a1.a_sym)
			a1.a_value &= 0xFF;
		outrab(&a1);
		break;
	case TWORD:
		outab(opcode);
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outraw(&a1);
		break;
	case TLONG:
		/* This is a bit of a fudge for now.. really wee need to
		   add long types to the asm core */
		outab(opcode);
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outraw(&a1);
		comma();
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outraw(&a1);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
