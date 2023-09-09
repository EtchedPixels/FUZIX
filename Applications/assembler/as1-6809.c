/*
 * Z-80 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"


/*
 * Deal with the syntactic mess 6809 assembler has
 *
 * In some cases (JSR JMP and definitions - eg .word)
 * $ABCD means a constant everywhere else that is #ABCD
 */

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

static void constant_to_dp(ADDR *ap)
{
	if (ap->a_segment != ABSOLUTE || ap->a_sym)
		return;
	/* FIMXE: need to support .setdp */
	if (ap->a_value > 255)
		return;
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
uint8_t getaddr(ADDR *ap, uint8_t *extbyte, uint8_t size)
{
	int c;
	ADDR tmp;
	ADDR off;

	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;
	
	c = getnb();

	/* Immediate */
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
		return size;
	}
	
	/* [xx] */
	if (c == '[') {
		getaddr_sub(ap, extbyte, size);
		c = getnb();
		if (c != ']')
			err('s', SQUARE_EXPECTED);
		if ((ap->ap_type & TADDR) == TAEXT) {
			if (!(*extbyte & 0x80)) {
				/* Convert from 5bit to 8bit */
				size = 1;
				*extbyte &= 0x60;
				*extbyte |= 0x88;	/* 8 bit offset */
			}
			c = (*extbyte & 0x9F);
			/* Not allowed to indirect a - / + just -- and ++ */
			if (c == 0x80 || c == 0x82)
				err('a', INVALID_INDIR);
			*extbyte |= 0x10;
			ap->a_type |= TAEXT|TMINDIR;
			return size;
		}
		return;
	}
	unget(c);
	getaddr_sub(ap, extbyte, size, &off);
}

/*
 *	Perform the main work of decoding modes that can be direct or
 *	indrected
 */
uint8_t getaddr_sub(ADDR *ap, uint8_t *extbyte, ADDR *off)
{
	int reg;
	int c;
	ADDR tmp;
	ADDR off;
	int index = 0;

	/* ,foo is the same as 0,foo */	
	off.a_type = TUSER;
	off.a_value = 0;

	/* We need to decode
	
		reg, reg
		offset,reg
		address
		dp:address
		autodec(2) reg
		reg autoinc(2)
		offset,pc
	 */
	c =  getnb();
	if (c != ',') {
		/* The left of the comma can be a constant or A/B/D register */
		unget(c);
		expr1(&off, LOPRI, 0);
		ireg = reg_expr(ap);
		if (reg != -1) {
			if (ireg != A || ireg != B || ireg != D)
				error(INVALID_REGISTER);
			index = 2;
			/* We are doing reg,index */
		} else { /* Must be a constant */
			istuser(&off);
			if (ap->a_value != 0)
				index = 1;
			c = get();
			if (c != ',') {
				/* Extended or DP addressing - lda foo */
				unget(c);
				/* FIXME figure out extbyte etc  */
				ap->a_type = TUSER;
				ap->a_value = off.a_value;
				ap->a_sym = off.a_sym;
				ap->a_segment = off.a_segment;
				return;
			}
			unget(c);
		}
		comma();
	}

	/* Afer the comma should be a register but it may have a - -- prefix */	
	c = getnb();
	if (c == '-') {
		if (index)
			aerr(BAD_MODE);
		extop = 0x82;
		c = get();
		if (c == '-')
			extop = 0x83;
		else
			unget(c);
	} else
		unget(c);

	/* Compute the expression that follows. Should be a register  */
	expr1(ap, LOPRI, 1);

	reg = reg_expr(ap);
	if (reg == -1)
		aerr(BAD_MODE);
	c = get();
	if (c == '+') {
		if (extop || index)	
			aerr(BAD_MODE);
		c = get();
		extop = 0x81;
		if (c == '+')
			extop = 0x80;
		else
			unget(c);
	} else
		unget(c);

	if (index == 2) {
		/* encode a/b/d, reg */
		switch(ireg)
		{
			case A:
				*extbyte = 0x86;
				break;
			case B:
				*extbyte = 0x85;
				break;
			case D:
				*extbyte = 0x8B;
				break;
			default:
				qerr(SYNTAX_ERROR);
		}
		*extbyte |= toindex(reg);
		return 0;
	}
	if (extop) {
		/* No offset with increment/decrement */	
		if (index)
			aerr(BAD_MDOE);
		/* --/-/+/++ */
		*extbyte = extop | toindex(reg);
		return 0;
	}
	/* Simple offset. Only it's not so simple because it might be a
	   symbol that is external. In which case we must assume the worst
	   FIXME: check this logic works for TNEW */

	if (ap->a_segment != ABSOLUTE)
		off = ap->a_value - dot[segment];

	/* Is this a relocation that we can compute te value of ? */
	if (ap->a_sym == NULL || (ap->a_type & TMMODE) == TUSER) {
		/* Is it within our segment or absolute */
		if (ap->a_segment == segment || ap->a_segment == ABSOLUTE) {
			/* In which case we can actually use the short forms */
			if (reg != PCR && offset5bit(off)) {
				*extbyte = offset5bit(off)|toindex(reg);
				return 0;
			}
			if (offset8bit(off)) {
				if (reg == PCR)
					*extbyte = 0x8C;
				else
					*extbyte = 0x88 | toindex(reg);
				return 1;
			}
		}
	}
	/* Otherwise we use the long form to be safe for linking */
	if (reg == PCR)
		*extbyte = 0x8D;
	else
		*extbyte = 0x89;
	return 2;
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

	case TBRA:
		/* FIXME: sort out symbols here and on 6502 */
		getaddr(&a1);
		disp = a1.a_value-dot[segment]-2;
		if (disp<-128 || disp>127 || a1.a_segment != segment)
			aerr(BRA_RANGE);
		outab(opcode);
		outab(disp);
		break;

	case TLBRA:
		/* FIXME: support this with symbols */
		getaddr(&a1);
		disp = a1.a_value-dot[segment]-2;
		if (disp<-32768 || disp> 32767 || a1.a_segment != segment)
			aerr(LBRA_RANGE);
		outab(opcode);
		outab(disp);
		break;

	case TIMPL:
		if (opcode >> 8)
			outab(opcode >> 8);
		outab(opcode);
		break;

	case TLO:
		size = getaddr(&a1, &ext);

		if (opcode >> 8)
			outab(opcode >> 8);
		switch(a1.a_type & TMADDR) {
			case 0:
				aerr(NO_IMMEDIATE);
				break;
			case TEXT:
				outab(opcode + 0x70);
				outraw(&a1);
				break;
			case TDP:
				outab(opcode);
				outrab(&a1);
				break;
			case TIND:
				outab(opcode + 0x60);
				outrab(ext);
				if (size == 1)
					outrab(&a1);
				else if (size == 2)
					outraw(&a1);
		}
		break;				
	case TLEA:
		size = getaddr(&a1, &ext);
		if ((a1.a1_type & TMADDR) != TEXT)
			aerr(MUST_BE_INDEXED);
		outab(opcode);
		outab(ext);
		if (size == 1)
			outrabs(a1);
		else if (size == 2)
			outraws(a1);
		break;
	case THI:
	case THIW:
	case THINOIMM:
	case THIWNOIMM:
		size = getaddr(&a1, &ext);
		mode = a1.a1_type & TMADDR;

		if (opcode >> 8)
			outab(opcode >> 8);

		switch(mode) {
			case 0:
				outab(opcode);
				switch(sp->s_type & TMMODE) {
				case THIW:
					outraw(&a1);
					break;
				case THI:
					outrab(&a1);
					break;
				default:
					aerr(NO_IMMEDIATE);
				}
				break;
			case TEXT:
				outab(opcode + 0x30);
				outraw(&a1);
				break;
			case TDP:
				outab(opcode + 0x10);
				outrab(&a1);
				break;
			case TIND:
				outab(opcode + 0x20);
				outrab(ext);
				if (size == 1)
					outrabs(&a1);
				else if (size == 2)
					outraws(&a1);
		}
		break;

	case TIMM8:
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value > 255)
			aerr(CONSTANT_TOO_LARGE);
		if (opcode >> 8)
			outab(opcode >> 8);
		outab(opcode);
		outrab(a1);
		break;

	case TEXG:
		r1 = get_register();
		comma();
		r2 = get_register();
		opcode |= ????
		outab(opcode);
		break;
	
	case TPUSH:
		mask |= 1 << get_register_mask();
		while((c = getnb()) == ',') {
			mask |= 1 << get_register_mask();
		unget(c);
		outab(opcde);
		outab(mask);
		break;
	
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
