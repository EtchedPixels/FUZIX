/*
 * TMS9995 assembler.
 * Assemble one line of input.
 */
#include	"as.h"

static int cputype;
/* FIXME: we should malloc/realloc this on non 8bit machines */
static uint8_t reltab[1024];
static unsigned int nextrel;

/*
 *	Set up for the start of each pass
 */
int passbegin(int pass)
{
	segment = 1;		/* Default to code */
	if (pass == 3)
		nextrel = 0;
	return 1;
}

static void setnextrel(int flag)
{
	if (nextrel == 8 * sizeof(reltab))
		aerr(TOOMANYJCC);
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

	/* Address descriptors are really simple as the instruction
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


static unsigned int gen4bit(void)
{
	ADDR ap;

	ap.a_type = 0;
	ap.a_flags = 0;
	ap.a_sym = NULL;

	expr1(&ap, LOPRI, 0);
	istuser(&ap);
	constify(&ap);
	if (ap.a_value < 0 || ap.a_value > 15)
		aerr(CONSTANT_RANGE);
	return ap.a_value;
}

static int gensigned8(void)
{
	ADDR ap;

	ap.a_type = 0;
	ap.a_flags = 0;
	ap.a_sym = NULL;

	expr1(&ap, LOPRI, 0);
	istuser(&ap);
	constify(&ap);
	if (ap.a_value < -128 || ap.a_value > 127)
		aerr(CONSTANT_RANGE);
	return ap.a_value;
}


/* R0 to R15, constants only */
static uint16_t wreg(void)
{
	int c, c2;

	c = getnb();
	if (c != 'r')
		aerr(REG_REQUIRED);
	/* We cannot use expr here - firstly it can't really be an
	   expression meaningfully, secondly we don't want to confuse the
	   trailing '+' syntax */
	c = getnb();
	if (!isdigit(c))
		qerr(SYNTAX_ERROR);
	c -= '0';
	c2 = getnb();
	if (isdigit(c2)) {
		c *= 10;
		c += c2 - '0';
	} else
		unget(c2);
	if (c > 15)
		aerr(REG_RANGE);
	return c;
}

/* R0, or 0-15 */
static uint16_t genshift(void)
{
	ADDR ap;
	int has_r = 0;
	int c;

	ap.a_type = 0;
	ap.a_flags = 0;
	ap.a_sym = NULL;

	c = getnb();
	if (c == 'r')
		has_r = 1;
	else
		unget(c);

	expr1(&ap, LOPRI, 0);
	istuser(&ap);
	constify(&ap);
	if (ap.a_value < 0 || ap.a_value > 15)
		aerr(CONSTANT_RANGE);
	if (ap.a_value && has_r)
		aerr(REG_ZEROONLY);
	return ap.a_value;
}

/* The following syntaxes are permitted

                Rn			0-15
                *Rn			0-15
                *Rn+			0-15
                @foo
                @foo[Rn]                1-15 */
                
static uint16_t genregaddr(int *immw, ADDR *addr)
{
	uint16_t tmp;
	int c;	

	*immw = 0;

	c = getnb();
	if (c == '@') {
		*immw = 1;
		getaddr(addr);
		c = get();
		if (c == '(') {
			/* @LABEL(R) */
			tmp = wreg();
			/* @LABEL(R0) not allowed */
			if (tmp == 0)
				aerr(REG_NOTZERO);
			c = get();
			if (c != ')')
				qerr(SYNTAX_ERROR);
			return 0x20 | tmp;
		}
		unget(c);
		return 0x20;
	}
	if (c != '*') {
		unget(c);
		/* Rn */
		return wreg();
	
	}
	/* *Rn or *Rn+ */

	tmp =  wreg();
	/* We should check for a trailing '+' need to see how that messes
	   with expression handler though */
	c = getnb();
	if (c == '+')
		tmp |= 0x30;
	else {
		tmp |= 0x10;
		unget(c);
	}
	return tmp;
}

/*
 *	A value for a jump instruction. This is an odd case
 *	JMP n is an offset, JMP @n is an address
 */

void getaddr_jmp(ADDR *ap, int *rel)
{
	int c = getnb();
	if (c != '@')
		unget(c);

	getaddr(ap);

	if (c != '@') {
		istuser(ap);
		/* JMP label without @ is nonsense unless its an absolute
		   value (ie a .equ) */
		if (ap->a_sym || ap->a_segment != ABSOLUTE)
			qerr(MUST_BE_ABSOLUTE);
		*rel = 1;
	} else {
		*rel = 0;
	}
}

/* You can't relative branch between segments */
static int segment_incompatible(ADDR *ap)
{
	if (ap->a_segment == segment)
		return 0;
	return 1;
}

static void outarg(ADDR *a1)
{
	constify(a1);
	istuser(a1);
	outraw(a1);
}

/* This is a bit of an oddity. It's a word machine so technically has no
   endianness but we have to pick one. We use BE because that is how 8bit
   bus variants of the CPU order bytes */
static void outaw(uint16_t val)
{
	if (dot[segment] & 1)
		aerr(ALIGNMENT);
	outab(val >> 8);
	outab(val);
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
	ADDR a2;
	int imm1, imm2;
	int disp;

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
		} else if (pass !=3) {
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
		if (dot[segment] & 1)
			aerr(ALIGNMENT);
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

		/* When we do jump expansion this will be ok because they
		   are always word aligned and word sized */
	case TEVEN:
		if (dot[segment] & 1)
			outab(0);	/* Zero so it works in BSS */
		break;

	case TIMPL:
		outaw(opcode);
		break;
	case TDOMA:
		opcode |= genregaddr(&imm1, &a1);
		comma();
		opcode |= genregaddr(&imm2, &a2) << 6;
		outaw(opcode);
		if (imm1)
			outarg(&a1);
		if (imm2)
			outarg(&a2);
		break;
	case TDOMAW:
		opcode |= genregaddr(&imm1, &a1);
		comma();
		opcode |= wreg() << 6;
		outaw(opcode);
		if (imm1)
			outarg(&a1);
		break;
	case TSMD:
		outaw(genregaddr(&imm1, &a1) | opcode);
		if (imm1)
			outarg(&a1);
		break;
	case TXOP:
		opcode |= genregaddr(&imm1, &a1);
		comma();
		opcode |= gen4bit() << 6;
		outaw(opcode);
		if (imm1)
			outarg(&a1);
		break;
	case TSOP:
		opcode |= genregaddr(&imm1, &a1);
		outaw(opcode);
		if (imm1)
			outarg(&a1);
		break;
	case TCRUM:
		opcode |= genregaddr(&imm1, &a1);
		comma();
		opcode |= gen4bit() << 6;
		outaw(opcode);
		if (imm1)
			outarg(&a1);
		break;
	case TCRUS:
		opcode |= gensigned8();
		outaw(opcode);
		break;
	case TJUMP:
		getaddr_jmp(&a1, &imm1);
		if (imm1)
			disp = a1.a_value;
		else
			disp = a1.a_value - dot[segment] - 2;
			/* Disp is in words */
		disp >>= 1;
		if (pass == 3 && (disp < -128 || disp > 127 || (!imm1 && segment_incompatible(&a1))))
			aerr(BRA_RANGE);
		opcode |= (uint8_t)disp;
		outaw(opcode);
		break;
	/* There is no JNP or JOO nor JGE/JLE signed comparison */
	/* For these LJxx always synthesizes the reversed form over a B @n */
	case TLJONLY:	/* Synthetic branches that must always be in skip form */
		getaddr_jmp(&a1, &imm1);
		/* An immediate in this case wants turning into a true addr */
		if (imm1)
			disp = a1.a_value + dot[segment] - 2;
		outaw(opcode | 2);	/* The reversed branch */
		outaw(0x0460);		/* BRA @n */
		if (imm1)
			outaw(disp);
		else
			outraw(&a1);
		break;
	case TLJUMP:	/* Relative branch or reverse and jump for range */
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
		getaddr_jmp(&a1, &imm1);
		if (imm1)	/* User specified as displacement */
			disp = a1.a_value;
		else		/* User specified symbolic */
			disp = a1.a_value - dot[segment] - 2;
		/* disp may change between pass1 and pass2 but we know it won't
		   get bigger so we can be sure that we still fit the 8bit disp
		   in pass 2 if we did in pass 1 */
		disp >>= 1;
		/* For pass 0 assume the worst case. Then we optimize on
		   pass 1 when we know what may be possible */
		if (pass == 3)
			c = getnextrel();
		else {
			c = 0;
			/* Cases we know it goes big */
			if (pass == 0 || (!imm1 && segment_incompatible(&a1)) || disp < -128 || disp > 127)
				c = 1;
			/* On pass 2 we lock down our choices in the table */
			if (pass == 2)
				setnextrel(c);
		}
		/* This is harder than many other processors because we may
		   have a branch that has no single conditional form in
		   reverse. We encode those in the two bytes ands write a short
		   essay - so for example LGJT x inverts to

				JEQ 3
				JLT 2
				B @x
		*/
		if (c) {
			static uint16_t brarev[16] = {
				/* JMP */ 	0xFFFF, /* special case */
				/* JLT: */	0x1513,
				/* JLE: JH */ 	0x1B00,
				/* JEQ: JNE */	0x1600,

				/* JHE: JL */	0x1A00,
				/* JGT: */	0x1113,
				/* JNE: JEQ */	0x1300,
				/* JNC: JOC */	0x1800,

				/* JOC: JNC */	0x1700,
				/* JNO: ??? */	0x0000,
				/* JL : JHE */	0x1400,
				/* JH : JLE */  0x1200,

				/* JOP : ?? */	0x0000,
				/* Not used */	0x0000,
				/* Not used */	0x0000,
				/* Not used */	0x0000,
			};

			opcode = brarev[(opcode & 0x0F00) >> 8];
			if (opcode == 0)
				qerr(BRA_BAD);
			if (opcode != 0xFFFF) {
				if (opcode & 0xFF) {
					/* Write the needed JEQ 3 */
					outab(opcode);
					outab(3);
				}
				/* Write the reversed JCC 2 */
				outab(opcode >> 8);	/* Inverted branch */
				outab(2);		/* Skip over the jump */
			}
			/* Write the B @n form */
			outaw(0x0460);
			/* If the user used an immediate form then they wrote
			   Jxx nn where n is relative to the PC after the Jxx
			   opcode and is not relocatable */
			if (imm1) {
				outaw(a1.a_value + dot[segment]);
			} else
				outraw(&a1);
		} else {
			/* Should never happen */
			if (disp < -128 || disp > 127)
				aerr(BRA_RANGE);
			opcode |= (uint8_t)disp;
			outaw(opcode);
		}
		break;

	case TSHIFT:
		opcode |= wreg();
		comma();
		opcode |= genshift() << 4;
		outaw(opcode);
		break;
	case TIMM:
		opcode |= wreg();
		comma();
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outaw(opcode);
		outraw(&a1);
		break;
	case TIRL:
		outaw(opcode);
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outraw(&a1);
		break;
	case TIRLS:
		opcode |= wreg();
		outaw(opcode);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
