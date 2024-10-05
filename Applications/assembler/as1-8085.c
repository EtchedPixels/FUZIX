/*
 * 8080/8085 assembler.
 * Assemble one line of input.
 */
#include	"as.h"

static int cputype = 8080;

static void require_8085(void)
{
	if (!(cpu_flags & OA_8080_8085)) {
		if (pass == 3)
			cpu_flags |= OA_8080_8085;
		if (cputype != 8085)
			err('1', REQUIRE_8085);
	}
}

/*
 *	Set up for the start of each pass
 */
int passbegin(int pass)
{
	cputype = 8080;
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

	/* Address descriptors in 8080 are really simple as the instruction
	   always explicitly implies the following type */
	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;

	c = getnb();

	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);
	expr1(ap, LOPRI, 0);
}

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

int xlat_reg(ADDR *ap, int bad)
{
	int reg;
	switch((ap->a_type & TMMODE))
	{
		case TWR:
			/* AF or SP encoding 3. Only one of the two is valid
			   for any given instruction - PSW for stack ops, SP
			   for anything else */
			if (ap->a_value == bad)
				aerr(INVALID_REG);
			return 3;
		case TBR:
			/* B D or H for BC DE HL pairs */
			reg = ap->a_type & TMREG;
			if (reg & 1)
				aerr(INVALID_REG);
			if (reg > H)
				aerr(INVALID_REG);
			return reg >> 1;
		default:
			aerr(INVALID_REG);
			return 0;
	}
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
			outab(0);
		break;

	case TSETCPU:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_value != 8080 && a1.a_value != 8085)
			aerr(SYNTAX_ERROR);
		cputype = a1.a_value;
		break;

	case TIMPL85:
		require_8085();
		/* Fall through */
	case TIMPL:
		outab(opcode);
		break;
	case TRST:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_value < 0 || a1.a_value > 7)
			aerr(INVALID_CONST);
		else
			outab(opcode|(a1.a_value << 3));
		break;
	case TI8_85:
		require_8085();
	case TI8:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outab(opcode);
		outrab(&a1);
		break;
	case TI16_85:
		require_8085();
	case TI16:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outab(opcode);
		outraw(&a1);
		break;
	case TREG8H:
		getaddr(&a1);
		reg = a1.a_type & TMREG;
		if ((a1.a_type & TMMODE) == TBR)
			outab(opcode | (reg << 3));
		else
			aerr(INVALID_REG);
		break;
	case TREG8:
		getaddr(&a1);
		reg = a1.a_type & TMREG;
		if ((a1.a_type & TMMODE) == TBR)
			outab(opcode | reg);
		else
			aerr(INVALID_REG);
		break;
	case TREG16:
		getaddr(&a1);
		reg = xlat_reg(&a1, PSW);
		outab(opcode | (reg << 4));
		break;
	case TREG16_P:
		getaddr(&a1);
		reg = xlat_reg(&a1, SP);
		outab(opcode | (reg << 4));
		break;
	case TREG16BD:
		getaddr(&a1);
		reg = xlat_reg(&a1, SP);
		if (reg > 1)
			aerr(INVALID_REG);
		outab(opcode | (reg << 4));
		break;
	case TMOV:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if ((a1.a_type & TMMODE) != TBR ||
		   ((a2.a_type & TMMODE) != TBR))
		   	aerr(INVALID_REG);
		if ((a1.a_type & TMREG) == M && (a2.a_type & TMREG) == M)
			aerr(INVALID_REG);
		outab(opcode | ((a1.a_type & TMREG) << 3) | (a2.a_type & TMREG));
		break;
	case TREG8_I8:
		getaddr(&a1);
		reg = a1.a_type & TMREG;
		if ((a1.a_type & TMMODE) == TBR)
			outab(opcode| (reg << 3));
		else
			aerr(INVALID_REG);
		comma();
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outrab(&a1);
		break;
	case TREG16_I16:
		getaddr(&a1);
		reg = xlat_reg(&a1, PSW);
		outab(opcode | (reg << 4));
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
