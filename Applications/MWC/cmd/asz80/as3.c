/*
 * Z-80 assembler.
 * Read in expressions in
 * address fields.
 */
#include	"as.h"

#define	LOPRI	0
#define	ADDPRI	1
#define	MULPRI	2
#define	HIPRI	3

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

	if ((c=getnb()) != '(') {
		unget(c);
		expr1(ap, LOPRI, 0);
		return;
	}
	expr1(ap, LOPRI, 1);
	if (getnb() != ')')
		qerr();
	reg = ap->a_type&TMREG;
	switch (ap->a_type&TMMODE) {
	case TBR:
		if (reg != C)
			aerr();
		ap->a_type |= TMINDIR;
		break;
	case TSR:
	case TCC:
		aerr();
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
			aerr();
		else
			ap->a_type |= TMINDIR;
	}
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
	int opri;
	ADDR right;

	expr2(ap);
	while ((c=getnb())=='+' || c=='-' || c=='*' || c=='/') {
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
			ap->a_value += right.a_value;
			break;
		case '-':
			istuser(&right);
			isokaors(ap, paren);
			ap->a_value -= right.a_value;
			break;
		case '*':
			istuser(ap);
			istuser(&right);
			ap->a_value *= right.a_value;
			break;
		case '/':
			istuser(ap);
			istuser(&right);
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
	if (c == '[') {
		expr1(ap, LOPRI, 0);
		if (getnb() != ']')
			qerr();
		return;
	}
	if (c == '-') {
		expr1(ap, HIPRI, 0);
		istuser(ap);
		ap->a_value = -ap->a_value;
		return;
	}
	if (c == '~') {
		expr1(ap, HIPRI, 0);
		istuser(ap);
		ap->a_value = ~ap->a_value;
		return;
	}
	if (c == '\'') {
		ap->a_type  = TUSER;
		ap->a_value = get();
		while ((c=get()) != '\'') {
			if (c == '\n')
				qerr();
			ap->a_value = (ap->a_value<<8) + c;
		}
		return;
	}
	if (c>='0' && c<='9') {
		expr3(ap, c);
		return;
	}
	if (isalpha(c)) {
		getid(id, c);
		if ((sp=lookup(id, uhash, 0)) == NULL
		&&  (sp=lookup(id, phash, 0)) == NULL)
			sp = lookup(id, uhash, 1);
		mode = sp->s_type&TMMODE;
		if (mode==TBR || mode==TWR || mode==TSR || mode==TCC) {
			ap->a_type  = mode|sp->s_value;
			ap->a_value = 0;
			return;
		}
		if (mode == TNEW)
			uerr(id);
		ap->a_type  = TUSER;
		ap->a_value = sp->s_value;
		return;
	}
	qerr();
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
	value = 0;
	while (np2 < np1) {
		if ((c = *np2++)>='0' && c<='9')
			c -= '0';
		else if (c>='a' && c<='f')
			c -= 'a'-10;
		else
			err('n');
		if (c >= radix)
			err('n');
		value = radix*value + c;
	}
	ap->a_type  = TUSER;
	ap->a_value = value;
}

/*
 * Make sure that the
 * mode and register fields of
 * the type of the "ADDR" pointed to
 * by "ap" can participate in an addition
 * or a subtraction.
 */
void isokaors(ADDR *ap, int paren)
{
	int mode;
	int reg;

	mode = ap->a_type&TMMODE;
	if (mode == TUSER)
		return;
	if (mode==TWR && paren!=0) {
		reg = ap->a_type&TMREG;
		if (reg==IX || reg==IY)
			return;
	}
	aerr();
}
