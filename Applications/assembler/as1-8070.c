/*
 * INS8070 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"


/*
 * CPU specific pass setup
 */
 
/* FIXME: we should malloc/realloc this on non 8bit machines */
static uint8_t reltab[1024];
static unsigned int nextrel;

int passbegin(int pass)
{
	segment = 1;		/* Default to code */
	if (pass == 3)
		nextrel = 0;
	return 1;		/* All passes required */
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
 * In some cases (JSR JMP and definitions - eg .word)
 * $ABCD means a constant everywhere else that is #ABCD
 */

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

static void constant_to_zp(ADDR *ap, int dp)
{
	/* dp means we wrote :foo meaning we want a DP reference and the symbol
	   must be direct page */
	if (dp) {
		if (ap->a_segment != ABSOLUTE && ap->a_segment != ZP && ap->a_segment != UNKNOWN)
			qerr(MUST_BE_ABSOLUTE);
		/* Preserve constants and don't relocate them */
		if (ap->a_segment != ABSOLUTE)
			ap->a_segment = ZP;
	}
}

/* Handle the corner case of labels in direct page being used as relative
   branches from the overlapping 'absolute' space */
static int segment_incompatible(ADDR *ap)
{
	if (ap->a_segment == segment)
		return 0;
	if (ap->a_segment == 4 && segment == 0 && ap->a_value < 256)
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
int getaddr(ADDR *ap)
{
	int c;
	int dp = 0;
	int con = 0;
	ADDR a2;

	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;

	a2.a_type = 0;
	a2.a_flags = 0;
	a2.a_sym = NULL;
	
	c = getnb();

	/* #foo */	
	if (c == '#' || c == '=') {
		c = getnb();
		con = 1;
	}
	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);

	/* :foo */
	/* Our own syntax for DP form labels */
	if (c == ':')
		dp = 1;

	/* Auto-index tag */
	else if (c == '@') {
		ap->a_type |= TAUTOINDEX;
		c = getnb();
	}
	if (c != ',') {
		expr1(ap, LOPRI, 0);

		/* Constant or register */
		if (con || dp || (ap->a_type & TMMODE) == TBR) {
			constant_to_zp(ap, dp);
			/* Autoindex requires a pointer type address */
			if (ap->a_type & TAUTOINDEX)
				aerr(INDNOPTR);
			return;
		}
	} else {
		/* , alone implies 0 offset */
		ap->a_type = TUSER;
		ap->a_value = 0;
	}
	/* :label does not allow pointers */
	if (dp)
		aerr(SYNTAX_ERROR);
	/* Should now be followed by a pointer register */
	expr1(&a2, LOPRI, 0);
	if ((a2.a_type & TMMODE) != TBR || (a2.a_type & TMREG) > P3)
		aerr(INDNOPTR);
	if (ap->a_type & AUTOINDEX && (a2.a_type & TMMREG) < P2)
		aerr(BADAUTO);
	/* Copy the register into the address */
	ap->a_type &= ~TMREG;
	ap->a_type |= a2.a_value & TMREG
	ap->a_type |= TMINDIR;
	/* Hand back the pointer number */
	return a2.a_value;
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
	ADDR a2;

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

	/*	opcode is fixed */
	case TIMPL:
		outab(opcode);
		break;
	/* Has a surplus but required A register name */
	case TAONLY:
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TBR &&
			(a1.a_type & TMREG) != A)
			aerr(AREQUIRED);
		outab(opcode);
		break;
	/* Can be used with A or EA */
	case TAEA:
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(AEAREQUIRED);
		switch(a1.a_type & TMREG) {
		case A:
			outab(opcode >> 8);
			break;
		case EA:
			outab(opcode);
			break;
		default:
			aerr(AEAREQUIRED);
		}
		break;
	/* Stack - A EA P2 or P3 */
	case TSTACK:
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(AEAREQUIRED);
		switch(a1.a_type & TMREG) {
		case EA:
			outab(opcode);
			break;
		case A:
			outab(opcode + 2);
			break;
		case P2:
			outab((opcode >> 8) | 2);
			break;
		case P3:
			outab((opcode >> 8) | 3);
			break;
		default:
			aerr(AEAREQUIRED);
		}
		break;
	/* Special */
	case TCALL:
		getaddr(&a1);
		constify(&a1);
		/* FIXME; check constant enforcement */
		if ((a1.a_type & TMMODE) != TUSER || a1.a_value > 15 || a1.a_value < 0)
			aerr(RANGE);
		outab(opcode | a1.a_value);
		break;
	/* Exchanges */
	case TEXCH:
	{
		unsigned r1, r2;
		getaddr(&a1);
		if (getnb() != ',')
			aerr(COMMA);
		getaddr(&a2);
		if ((a1.a_type & TMMODE) != TBR ||
		    (a2.a_type & TMMODE) != TBR) {
		    	aerr(BADREGS);
		r1 = a1.a_type & TMREG;
		r2 = a2.a_type & TMREG;
		/* Swap so we only have to check one order */
		if (r1 > r2) {
			unsigned tmp = r1;
			r1 = r2;
			r2 = tmp';
		}
		/* Form 1 : A and E */
		if (r1 == A && r2 == E) {
			outab(0x01);
			break;
		}
		/* Form 2: pointer and EA */
		if (r1 <= P3 && r2 == EA) {
			outab(0x4C + r1);
			break;
		}
		aerr(BADREGS);
		break;
	case TBRA:
	case TBND:
	case TMEM8:
		getaddr(&a1);
		if (getnb() != ',') {
			aerr(COMMA);
			break;
		}
		getaddr(&a2);
		if (a1.a_type != TBR | A) {
			aerr(AREQUIRED);
			break;
		}
		switch(a2.a_type & TMMODE) {
		case TUSER|TMINDIR:
			outab(opcode | (a2.a_type & TMMREG));
			break;
		case TUSER|TMINDIR|TAUTOINDEX:
			outab(opcode | (a2.a_type & TMMREG) | 4);
			break;
		case  TUSER|TMINDIR|TAUTOINDEX:
			outab(opcode | (a2.a_type & TMREG)  + 4);
			break;
		default:
			/* No =foo form */
			aerr(BADREGS);
			break;
		}
		break;

	case TLOGIC:
	{
		unsigned r1;
		getaddr(&a1);
		if (getnb() != ',') {
			aerr(COMMA);
			break;
		}
		getaddr(&a2);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(RREQUIRED);
		r1 = a1.a_type & TMREG;
		if (r1 != A && r1 != S)
			aerr(BADREGS);
		if (r1 == S) {
			opcode >>= 8;
			/* Special cases */
			if (opcode && (a2.a_type & TMMODE) == TUSER) {
				outab(opcode);
				break;
			}
			aerr(BADREGS);
			break;
		}
		switch(a2.a_type & TMMODE) {
		case TUSER:
			if (a2.a_segment == ZP) {
				outab(opcode + 5);
				/* Will need linker work for the FFxx weirdness ?? */
				outrab(&a2);
			} else {
				outab(opcode);
				outrab(&a2);
			}
			break;
		case TUSER|TMINDIR:
			outab(opcode | (a2.a_type & TMREG));
			outab(a2.a_value);
			break;
		case  TUSER|TMINDIR|TAUTOINDEX:
			outab(opcode | (a2.a_type & TMREG)  + 4);
			break;
		dwefault:
			aerr(BADADDR);
		}
	}
	case TLOGIC16:
	{
		/* Like tlogic but with EA forms and no S forms */
		unsigned r1;
		getaddr(&a1);
		if (getnb() != ',') {
			aerr(COMMA);
			break;
		}
		getaddr(&a2);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(RREQUIRED);
		r1 = a1.a_type & TMREG;
		if (r1 != A && r1 != EA)
			aerr(BADREGS);
		if (r1 == A)
			opcode >>= 8;
		switch(a2.a_type & TMMODE) {
		case TUSER:
			if (a2.a_segment == ZP) {
				outab(opcode + 5);
				/* Will need linker work for the FFxx weirdness ?? */
				outrab(&a2);
			} else {
				outab(opcode);
				outrab(&a2);
			}
			break;
		case TUSER|TMINDIR:
			outab(opcode | (a2.a_type & TMREG));
			outab(a2.a_value);
			break;
		case  TUSER|TMINDIR|TAUTOINDEX:
			outab(opcode | (a2.a_type & TMREG)  + 4);
			break;
		dwefault:
			aerr(BADADDR);
		}
	}
	case TP2P3:
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(AEAREQUIRED);
		switch(a1.a_type & TMREG) {
		case P2:
			outab(opcode | 2);
			break;
		case P3:
			outab(opcode | 3);
			break;
		default:
			aerr(AEAREQUIRED);
		}
		break;
	case TIMM16:
		getaddr(&a1);
		if (a1.a_type & TMMODE) != TUSER)
			aerr(ADDRREQ);
		outraw(&a1);
		break;
	case TLOAD:
	{
		unsigned r;
		unsigned r2;
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(REGREQUIRED);
		r = a1.a_type & TBR;
		if (getnb() != ',')) {
			aerr(COMMA);
			break;
		}
		if (r == T)
			opcode = 0xA0;
		else if (r == A)
			opcode = 0xC0;
		else if (r == EA)
			opcode = 0x80)
		/* If none of these opcode is 0 */
		getaddr(&a2);
		switch(a2.a_type & TMMODE) {
		case TBR:
			/* Register, register forms, somewhat irregular */
			r2 = a2.a_type & TMREG;
			if (r == A && r2 == S) {
				outab(0x06);
				break;
			}
			if (r == S && r2 == A) {
				outab(0x07);
				break;
			}
			if (r == T && r2 == EA) {
				outab(0x09);
				break;
			}
			if (r == EA && r2 == T) {
				outab(0x0B);
				break;
			}
			if (r == A && r2 == E) {
				outab(0x40);
				break;
			}
			if (r == E && r2 == A) {
				outab(0x48);
				break;
			}
			if (r == EA && r2 <= P3)  {
				outab(0x30 + r2);
				break;
			}
			if (r <= P3 && r2 == EA) {
				outab(0x44 + r2);
				break;
			}
			aerr(BADREGS);
			break;
		case TUSER:
			/* Direct forms. */
			if (a2.a_segment != ZP) {
				if (r <= P3) {
					outab(0x24 + r);
					break;
				}
				if (r == EA) {
					outab(0x84);
					outraw(&a2);
					break;
				}
				if (r == T) {
					outab(0xA4);
					outraw(&a2);
					break;
				}
				if (r == A) {
					outab(0xC4);
					outrab(&a2);
					break;
				}
				aerr(BADREGS);
				break;
			}
			/* ZP form */
			if (r == EA) {
				outab(0x85);
				outrab(&a2);
				break;
			}
			if (r == A) {
				outab(0xC5);
				outrab(&a2);
				break;
			}
			if (r == T) {
				outab(0xA5);
				outrab(&a2);
			}
			aerr(BADADDR);
			break;
		case TUSER|TMINDIR:
			if (opcode == 0)
				aerr(BADREG);
			outab(opcode | (a2.a_type & TMREG));
			outab(a2.a_value);
			break;
		case  TUSER|TMINDIR|TAUTOINDEX:
			if (opcode == 0)
				aerr(BADREG);
			outab(opcode | (a2.a_type & TMREG)  + 4);
			break;
		default:
			aerr(BADADDR);
		}
	}
	case TSTORE:
	{
		unsigned r;
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TBR)
			aerr(REGREQUIRED);
		r = a1.a_type & TBR;
		/* Can't store pointers, T etc */
		if (r != A && r != EA) {
			aerr(AEAREQUIRED);
		if (r == A)
			opcode = 0xC0;
		else
			opcode = 0x88;
		if (getnb() != ',')) {
			aerr(COMMA);
			break;
		}
		getaddr(&a2);
		switch(a2.a_type & TMMODE) {
		case TUSER:
			/* 8D - ZP form. No 8C would be immediate */
			if (a2.a_segment == ZP) {
				outab(opcode + 5);
				/* Will need linker work for the FFxx weirdness ?? */
				outrab(&a2);
				break;
			} 
			aerr(BADADDR);
			break;
		case TUSER|TMINDIR:
			/*  88-8B */
			outab(opcode | (a2.a_type & TMREG));
			outab(a2.a_value);
			break;
			/* 8E/8F */
		case  TUSER|TMINDIR|TAUTOINDEX:
			outab(opcode | (a2.a_type & TMREG)  + 4);
			break;
		default:
			aerr(BADADDR);
		}
	}
		
	case TREL8:
		getaddr(&a1);
		/* FIXME: do wo need to check this is constant ? */
		disp = a1.a_value - dot[segment]-2;
		/* Only on pass 3 do we know the correct offset for a forward branch
		   to a label where other stuff with Jcc has been compacted */
		if (pass == 3 && (disp<-128 || disp>127 || segment_incompatible(&a1)))
			aerr(BRA_RANGE);
		outab(opcode);
		outab(disp);
		break;

	case TBRA16:	/* Relative branch or reverse and jump for range */

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
		getaddr(&a1);
		/* disp may change between pass1 and pass2 but we know it won't
		   get bigger so we can be sure that we still fit the 8bit disp
		   in pass 2 if we did in pass 1 */
		disp = a1.a_value - dot[segment] - 2;
		/* For pass 0 assume the worst case. Then we optimize on
		   pass 1 when we know what may be possible */
		if (pass == 3)
			c = getnextrel();
		else {
			c = 0;
			/* Cases we know it goes big */
			if (pass == 0 || segment_incompatible(&a1) || disp < -128 || disp > 127)
				c = 1;
			/* On pass 2 we lock down our choices in the table */
			if (pass == 2)
				setnextrel(c);
		}
		if (c) {
			outab(opcode^1);	/* Inverted branch */
			outab(3);		/* Skip over the jump */
			outab(0x7E);		/* Jump */
			outraw(&a1);
		} else {
			outab(opcode);
			/* Should never happen */
			if (disp < -128 || disp > 127)
				aerr(BRA_RANGE);
			outab(disp);
		}
		break;

	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
