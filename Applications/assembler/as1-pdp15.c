/*
 * PDP15 assembler.
 * Assemble one line of input.
 */
#include	"as.h"

static int cputype = 4;

/*
 *	Set up for the start of each pass
 */
int passbegin(int pass)
{
	cputype = 4;
	segment = 1;		/* Default to code */
	/* We have no variable sized branches to fix up so
	   we do not do pass 1 and 2 */
	if (pass == 1 || pass == 2)
		return 0;
	return 1;
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
	int c;

	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;

	/* A leading '*' means an indirect reference */
	c = getnb();
	if (c == '*')
		ap->a_flags |= A_INDIRECT;

	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);

	/* This should be an absolute 12 or 11 bit constant depending on
	   mode; TODO: dot opt for mode info and sanity checks */
	expr1(ap, LOPRI, 0);
	c = getnb();
	/* Indexed ? */
	if (c != ',') {
		if (index_number) {
			if (ap->a_value < 0 || ap->a_value > 03777)
				aerr(MEM_RANGE);
			ap->a_flags |= A_LOW12;
		}
		else {
			if (ap->a_value < 0 || ap->a_value > 07777)
				aerr(MEM_RANGE);
			ap->a_flags |= A_LOW13;
		}
		unget(c);
		return;
	}
	/* ,X should be present */
	if (index_mode == 0) {
		aerr(SYNTAX_ERROR);
		return;
	}
	c = getnb();
	/* TODO error on older CPU */
	if (c != 'X' && c != 'x')
		aerr(SYNTAX_ERROR);
	if (ap->a_value < 0 || ap->a_value > 03777)
		aerr(INDX_RANGE);
	ap->a_flags |= TINDEX;
}

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
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
	int reg;
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
			outaw(0);
		break;

	case TSETCPU:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_value != 4 && a1.a_value != 7 && a1.a_value != 9 &&
		     a1.a_value != 15)
			aerr(SYNTAX_ERROR);
		cputype = a1.a_value;
		break;

	case TMODE:
		if (opcode && cpu < 15)	/*9 or 15 ? */
			aerr(WRONGCPU);
		index_mode = opcode;
		break;

	case TIMPL7:
		if (cputype < 7)
			aerr(WRONGCPU);
		outaw(opcode);
		break;
	case TIMPL9:
		if (cputype < 9)
			aerr(WRONGCPU);
		outaw(opcode);
		break;
	case TIMPL15:
		if (cputype < 15)
			aerr(WRONGCPU);
		outaw(opcode);
		break;
	case TIMPLE:	/* For now assume EAE */
	case TIMPL:
		outaw(opcode);
		break;
	case TMEM:
		/* top 4 bits opcode, then indirect, indexed/banked, operand */
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		/* We page align so nothing here is relocatable */
		/* TODO: fix this to allow symbol low bits */
		if (index_mode)
			a1.a_value |= A_LOW12;
		else
			a1.a_value |= A_LOW13;
		outraw_op(opcode, &a1);
		break;
	case TLAW:
		/* Load negative constant */
		getaddr(&a1);
		constify(&a1);;
		istuser(&a1);
		a1.a_value |= A_LOW13;
		outraw_op(opcode, &a1);
		break;		
	case T9BIT:
		/* Opcode with 9bit constant attached */
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_value < 0 || a1.a_value > 0777)
			aerr(OPRANGE);
		/* Lives in the low 9bits */
		a1.a_value |= A_LOW;
		outraw_9bit(opcode, &a1);
		break;
		
	/* EAE */
	case TEAE:
	case TOPR:
	case TIOT:
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
