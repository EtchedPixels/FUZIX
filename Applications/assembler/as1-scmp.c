/*
 * SC/MP assembler.
 * Assemble one line of input.
 */
#include	"as.h"


int passbegin(int pass)
{
	segment = 1;		/* Default to code */
	/* We have no variable sized branches to fix up so
	   we do not do pass 1 and 2 */
	if (pass == 1 || pass == 2)
		return 0;
	return 1;		/* All passes required */
}

/*
 *	Make constant
 */

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

static int getindex(void)
{
	int c;
	c = getnb();
	if (c != 'p') {
		aerr(POINTER_REQ);
		return 0;
	}
	c = getnb();
	if (c == 'c')
		c = '0';
	if (c < '0' || c > '3')
		aerr(POINTER_REQ);
	return c - '0';
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
 * The following forms are permitted
 *
 * xxxx(pn)		- offset from pointer (signed 8)
 * @n(pn)		- ditto with post inc/predec
 * 
 * immediate (actually @0(pc)), but has its own instruction form to the user
 */
void getaddr(ADDR *ap)
{
	int c;

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
	constify(ap);
}

/*
 * Parse a full address descriptor. If autoi is NULL disallow auto-index
 */
void get_mpd(ADDR *ap, int *dp, int *autoi)
{
	int c;
	int autoindex = 0;

	c = getnb();
	if (c == '@') {
		autoindex = 1;
		if (autoi == NULL)
			aerr(NO_AUTOINDEX);
		c = getnb();
	}

	/* Not clear whether stuff like @<label(p1) is any use but it works so
	   allow for it */
	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);

	c = getnb();
	/* -128 means e so you can write e(p1) to mean 'offset in e from p1 */
	if (c == 'e')
		ap->a_value = -128;
	else {
		unget(c);
		getaddr(ap);
		if (ap->a_value < -127 || ap->a_value > 127)
			aerr(RANGE);
	}

	/* And there must be a pointer register. There is no 16bit immediate
	   form in the CPU at all */
	c = getnb();
	if (c == '(') {
		*dp = getindex();
		ap->a_type = TINDEX | TUSER;
		c = getnb();
		if (c != ')')
			aerr(SYNTAX_ERROR);
		return;
	} else
		aerr(SYNTAX_ERROR);
	if (autoi)
		*autoi = autoindex;
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
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;
	int ds;
	int autor;

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
			/* Catch duplicates on phase 0 */
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
			/* Phase 1 defined the values so a misalignment here
			   is fatal */
			if ((sp->s_type&TMMDF) != 0)
				err('m', MULTIPLE_DEFS);
			if (sp->s_value != dot[segment]) {
				printf("Phase 2: Dot %x Should be %x\n",
					dot[segment], sp->s_value);
				err('p', PHASE_ERROR);
			}
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
		/* TODO: double check this logic and test validity */
		/* On pass 1 we expect to see ourself in the mirror, jsut
		   update the value */
		if (pass != 1) {
			if ((sp->s_type&TMMODE) != TNEW
			&&  (sp->s_type&TMASG) == 0)
				err('m', MULTIPLE_DEFS);
		}
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
		/* FIXME: make a new common error, and push to other ports */
		if (((sp->s_type & TMMODE) == TNEW) && pass == 3)
			aerr(ADDR_REQUIRED);
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

	case TIMM8:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_value < -128 || a1.a_value > 127)
			aerr(RANGE);
		outab(opcode);
		outrab(&a1);
		break;

	case TMPD:
		get_mpd(&a1, &ds, &autor);
		outab(opcode | ds | (autor ? 4 : 0));
		outab(a1.a_value);
		break;

	case TPD:
		get_mpd(&a1, &ds, NULL);
		outab(opcode | ds);
		outab(a1.a_value);
		break;

	case TPTR:
		c = getindex();
		outab(opcode | c);
		break;

	case TREL8:
		getaddr(&a1);
		/* FIXME: do wo need to check this is constant ? */
		disp = a1.a_value - dot[segment]-2;
		/* Only on pass 3 do we know the correct offset for a forward branch
		   to a label where other stuff with Jcc has been compacted */
		if (pass == 3 && (disp<-128 || disp > 127))
			aerr(BRA_RANGE);
		outab(opcode);
		outab(disp);
		break;

	case TJS:
		ds = getindex();
		c = get();
		if (c != ',')
			aerr(SYNTAX_ERROR);
		getaddr(&a1);
		outab(0xC4);		/* LDI */
		a1.a_flags |= A_HIGH;
		outrab(&a1);		/* High byte */
		outab(0x34|ds);		/* XPAH p */
		outab(0xC4);		/* LDI */
		a1.a_flags &= ~A_HIGH;
		a1.a_flags |= A_LOW;
		outrab(&a1);		/* Low byte */
		outab(0x30|ds);		/* XPAL p */
		outab(0x3C|ds);		/* XPPC p */
		break;

	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
