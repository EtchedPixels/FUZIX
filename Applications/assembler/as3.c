/*
 * Z-80 assembler.
 * Read in expressions in
 * address fields.
 */
#include	"as.h"



/*
 * Check if the mode of
 * an ADDR is TUSER. If not, give
 * an error.
 */
void istuser(ADDR *ap)
{
	if ((ap->a_type&TMMODE) != TUSER)
		aerr(ADDR_REQUIRED);
}

/*
 * A segment needs to meet a rule. Make sure that we don't have
 * internmal inconsistency
 */
static void setsegment(SYM *s, int seg)
{
	if (seg == UNKNOWN)
		return;

	if (s->s_segment == UNKNOWN || s->s_segment == seg) {
		s->s_segment = seg;
		return;
	}
	aerr(SEGMENT_CLASH);
}

static void chkabsolute(ADDR *a)
{
	/* Not symbols, doesn't matter */
	if ((a->a_type & TMMODE) != TUSER)
		return;
	if (a->a_segment != ABSOLUTE)
		aerr(MUST_BE_ABSOLUTE);
}

static void chksegment(ADDR *left, ADDR *right, int op)
{
	uint16_t m = (left->a_type & TMMODE);
	if (m == TBR || m == TWR) {
		if (op != '+')
			aerr(MUST_BE_ABSOLUTE);
		if ((right->a_type & TMMODE) == TUSER) {
			left->a_segment = right->a_segment;
			return;
		}
		aerr(MUST_BE_ABSOLUTE);
	}
	/* Not symbols, doesn't matter */
	if ((left->a_type & TMMODE) != TUSER ||(right->a_type & TMMODE) != TUSER)
		return;

	/* Anything goes with absolutes */
	if (left->a_segment == ABSOLUTE && right->a_segment == ABSOLUTE)
		return;

	/* This relies on ABSOLUTE being 0, an addition of segment offset and
	   absolute either way around produces a segment offset */
	if ((left->a_segment == ABSOLUTE || right->a_segment == ABSOLUTE) &&
		op == '+') {
		if (left->a_sym)
			right->a_sym = left->a_sym;
		else if (right->a_sym)
			left->a_sym = right->a_sym;
		left->a_segment += right->a_segment;
		return;
	}
	/* Subtraction within segment produces an absolute */
	if (op == '-') {
		/* We can't currently represent this and it's hard because
		   we'd actually need to pass the expression tree to the
		   linker. Only allow known symbols so that you can
		   do a-b providing a and b are in your object module */
#if 0
		/* Unknown symbols may get segment forced as a result */
		if (left->a_segment == UNKNOWN) {
			left->a_segment = right->a_segment;
			if (left->a_sym)
				setsegment(left->a_sym, left->a_segment);
		}
		if (right->a_segment == UNKNOWN) {
			right->a_segment = left->a_segment;
			if (right->a_sym)
				setsegment(right->a_sym, right->a_segment);
		}
#else
		if (left->a_segment != UNKNOWN && right->a_segment != UNKNOWN)
#endif
		if (left->a_segment == right->a_segment && op == '-') {
			left->a_segment = ABSOLUTE;
			left->a_sym = NULL;
			return;
		}
#if 0
		/* - constant we can do. The left segment remains unchanged */
		/* Disable this until we have a negative tag for the linker
		   to do overflow processing correctly */
		if (right->a_segment == ABSOLUTE)
			return;
#endif
	}
	left->a_sym = NULL;
	aerr(MUST_BE_ABSOLUTE);
}

/*
 * Expression reader,
 * real work, part I. Read
 * operators and resolve types.
 * The "lpri" is the firewall operator
 * priority, which stops the scan.
 * The "paren" argument is true if
 * the expression is in parentheses.
 */
void expr1(ADDR *ap, int lpri, int paren)
{
	int c;
	int c2;
	int opri;
	ADDR right;

	expr2(ap);
	while ((c=getnb())=='+' || c=='-' || c=='*' || c=='/') {
		c2 = getnb();
		/* Pass unary postifx back to the caller as some assembly
		    syntaxes require we can handle stuff like (X-) for
		    their own post dec notations */
		if (c2 == ')') {
			unget(c2);
			break;
		}
		unget(c2);
		opri = ADDPRI;
		if (c=='*' || c=='/')
			opri = MULPRI;
		if (opri <= lpri)
			break;
		expr1(&right, opri, paren);
		switch (c) {
		case '+':
			if ((ap->a_type&TMMODE) != TUSER)
				istuser(&right);
			else
				ap->a_type = right.a_type;
			isokaors(ap, paren);
			chksegment(ap, &right, '+');
			ap->a_value += right.a_value;
			break;
		case '-':
			istuser(&right);
			isokaors(ap, paren);
			chksegment(ap, &right, '-');
			ap->a_value -= right.a_value;
			break;
		case '*':
			istuser(ap);
			istuser(&right);
			chksegment(ap, &right, '*');
			ap->a_value *= right.a_value;
			break;
		case '/':
			istuser(ap);
			istuser(&right);
			chksegment(ap, &right, '/');
			if (right.a_value == 0)
				err('z', DIVIDE_BY_ZERO);
			else
				ap->a_value /= right.a_value;
		}
	}
	unget(c);
}

/*
 * Expression reader,
 * real work, part II. Read
 * in terminals.
 */
void expr2(ADDR *ap)
{
	int c;
	SYM *sp;
	int mode;
	char id[NCPS];

	c = getnb();
#ifndef TARGET_USES_SQUARE
	if (c == '[') {
		expr1(ap, LOPRI, 0);
		if (getnb() != ']')
			qerr(SQUARE_EXPECTED);
		return;
	}
#else
	if (c == '(') {
		expr1(ap, LOPRI, 0);
		if (getnb() != ')')
			qerr(BRACKET_EXPECTED);
		return;
	}
#endif
	if (c == '-') {
		expr1(ap, HIPRI, 0);
		istuser(ap);
		chkabsolute(ap);
		ap->a_value = -ap->a_value;
		return;
	}
	if (c == '~') {
		expr1(ap, HIPRI, 0);
		istuser(ap);
		chkabsolute(ap);
		ap->a_value = ~ap->a_value;
		return;
	}
	if (c == '\'') {
		ap->a_type  = TUSER;
		ap->a_value = get();
		ap->a_segment = ABSOLUTE;
		while ((c=get()) != '\'') {
			if (c == '\n')
				qerr(PERCENT_EXPECTED);
			ap->a_value = (ap->a_value<<8) + c;
		}
		return;
	}
	if ((c>='0' && c<='9') || c == '$') {
		expr3(ap, c);
		return;
	}
	if (isalpha(c) || c == '_') {
		getid(id, c);
		if ((sp=lookup(id, uhash, 0)) == NULL
		&&  (sp=lookup(id, phash, 0)) == NULL)
			sp = lookup(id, uhash, 1);
		mode = sp->s_type&TMMODE;
		if (mode==TBR || mode==TWR || mode==TSR || mode==TCC) {
			ap->a_type  = mode|sp->s_value;
			ap->a_value = 0;
			ap->a_segment = UNKNOWN;
			return;
		}
		if (mode != TNEW && mode != TUSER)
			qerr(SYNTAX_ERROR);
		/* An external symbol has to be tracked and output in
		   the relocations. Any known internal symbol is just
		   encoded as a relocation relative to a segment */
		if (mode == TNEW) {
			uerr(id);
			ap->a_sym = sp;
		} else
			ap->a_sym = NULL;
		ap->a_type  = TUSER;
		ap->a_value = sp->s_value;
		ap->a_segment = sp->s_segment;
		return;
	}
	qerr(SYNTAX_ERROR);
}

/*
 * Read in a constant. The argument
 * "c" is the first character of the constant,
 * and has already been validated. The number is
 * gathered up (stopping on non alphanumeric).
 * The radix is determined, and the number is
 * converted to binary.
 */
void expr3(ADDR *ap, int c)
{
	char *np1;
	char *np2;
	int radix;
	VALUE value;
	char num[40];

		
	np1 = &num[0];
	do {
		if (isupper(c))
			c = tolower(c);
		*np1++ = c;
		c = *ip++;
	} while (isalnum(c));
	--ip;

	/* The grammar here is ambiguous in the case of $11b which should
	   be 11B hex, but 11b is 11 boolean. We must therefore handle the
	   $ case first */
	if (num[0] == '$') {
		np2 = &num[1];
		radix = 16;
	} else if (num[0] == '0' && num[1] == 'x') {
		np2 = &num[2];
		radix = 16;
	} else {
		/* Look for trailing information on the radix */
		switch (*--np1) {
		case 'h':
			radix = 16;
			break;
		case 'o':
		case 'q':
			radix = 8;
			break;
		case 'b':
			radix = 2;
			break;
		default:
			radix = 10;
			++np1;
		}
		np2 = &num[0];
		/* No trailing tag, so look for 0octab, 0xhex */
		if (radix == 10) {
			if (*np2 == '0') {
				radix = 8;
				np2++;
				if (*np2 == 'x') {
					radix = 16;
					np2++;
				}
			}
		}
	}
	value = 0;
	while (np2 < np1) {
		if ((c = *np2++)>='0' && c<='9')
			c -= '0';
		else if (c>='a' && c<='f')
			c -= 'a'-10;
		else
			err('n', INVALID_CONST);
		if (c >= radix)
			err('n', INVALID_CONST);
		value = radix*value + c;
	}
	ap->a_type  = TUSER;
	ap->a_value = value;
	ap->a_segment = ABSOLUTE;
	ap->a_sym = NULL;
}

