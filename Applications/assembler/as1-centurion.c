/*
 * Warrex Centurion
 *
 * Assemble one line of input.
 */
#include	"as.h"


/*
 * CPU specific pass setup
 */

/* FIXME: we should malloc/realloc this on non 8bit machines */
static uint8_t reltab[1024];
static unsigned nextrel;
static unsigned cpu_model;

int passbegin(int pass)
{
	segment = 1;		/* Default to code */
	cpu_model =  6;		/* Assume CPU-6 for now */
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


/*
 *	We support the following bits of syntax
 *
 *	regname			TBR or TWR
 *	constant		TUSER
 *
 *	There is a lot more syntax but it's specific to address descriptions
 *	in some instructions and best handled explicitly
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
}

/*
 *	Encode a full address description
 *
 *	constant
 *	(addr)
 *	@(addr)
 *	n(PC)
 *	@n(PC)
 *	(regpair)
 *	offset(regpair)
 *	offset(-regpair)
 *	offset(regpair+)
 */

void address_encoding(uint8_t opbase, unsigned type, unsigned store, unsigned size)
{
	int c = getnb();
	unsigned indir = 0, predec = 0, postinc = 0, offset = 0;
	ADDR a1;
	ADDR a2;

	if (c == '@') {
		indir = 1;
		c = getnb();
	}
	if (c != '(')  {
		unget(c);
		getaddr(&a1);
		c = getnb();
		if (c != '(') {
			unget(c);
			/* Constant */
			switch(a1.a_type & TMMODE) {
			case TUSER:
				if (indir)
					aerr(BADINDIR);
				if (store)
					aerr(BADADDR);
				/* Jumps load the generated address into the PC so
				   as written we need one more layer of indirection so
				   the user can write jump foo or jump (xx) not jump (foo)
				   and jump @(xx) */
				if (type == 2)
					outab(opbase | 1);
				else
					outab(opbase);
				if (size == 1)
					outrab(&a1);
				else
					outraw(&a1);
				return;
			default:
				aerr(BADADDR);
			}
		}
		istuser(&a1);
		offset = 1;
		/* Inside the bracket of some format */
		if (a1.a_value < (uint16_t)-128 && (int)a1.a_value > 127)
			aerr(INDX_RANGE);
	}
	c = getnb();
	if (c == '-')
		predec = 1;
	else
		unget(c);
	getaddr(&a2);
	c = getnb();
	if (c == '+') {
		postinc = 1;
		c = getnb();
	}
	if (c != ')')
		aerr(SYNTAX_ERROR);
	/* Now unpick the pieces */
	switch(a2.a_type & TMMODE) {
	case TBR:
		aerr(WREGONLY);
		break;
	case TWR:
		if (indir)
			aerr(BADINDIR);
		if (predec) {
			if ((a2.a_type & TMMODE) != TWR)
				aerr(WREGONLY);
			outab(opbase | 5);
			if (offset) {
				outab(((a2.a_type & TMREG) << 4) | 10);
				outab(a1.a_value);
			} else {
				outab(((a2.a_type & TMREG) << 4) | 2);
			}
			return;
		}
		if (postinc) {
			if ((a2.a_type & TMMODE) != TWR)
				aerr(WREGONLY);
			outab(opbase | 5);
			if (offset) {
				outab(((a2.a_type & TMREG) << 4) | 9);
				outab(a1.a_value);
			} else {
				outab(((a2.a_type & TMREG) << 4) | 1);
			}
			return;
		}
		/* Turn 0(x) into (x) */
		if (offset && a1.a_value == 0)
			offset = 0;
		/* Simply an index pair */
		switch(type) {
		case 0:	/* Instructions with 4 mode bits */
			if (!offset) {
				outab(opbase | 0x08 | ((a2.a_type & TMREG) >> 1));
				return;
			}
			/* Fall through */
		case 1:	/* Instructions with 3 mode bits - use the longer form */
		case 2: /* Branches */
			outab(opbase | 5);
			if (offset) {
				outab(((a2.a_type & TMREG) << 4) | 8);
				outab(a1.a_value);
			} else {
				outab(((a2.a_type & TMREG) << 4));
			}
			return;
		}
		break;
	case TSR:
		/* PC relative forms */
		if (predec || postinc)
			aerr(BADADDR);
		outab(opbase + 3 + indir);
		if (offset)
			outab(a1.a_value);
		else
			outab(0);
		break;
	case TUSER:
		/* Can't pre-dec or post inc (nn) */
		if (predec || postinc)
			aerr(BADADDR);
		/* Simple forms */
		if (indir == 1) {
			outab(opbase | 2);
			outraw(&a2);
		}
		outab(opbase | 1);
		outraw(&a2);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}			
}

static SYM branch_bl = {
	0,	"bl",		TREL8,		0x10
};

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
	unsigned r1, r2;
	unsigned force8 = 0;

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
	/* Fix up "bl" being ambiguous */
	if ((sp->s_type & TMMODE) == TBR && sp->s_value == RBL)
		sp = &branch_bl;

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

	case TSETCPU:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		switch(a1.a_value) {
		case 4:
			cpu_model = 4;
			break;
		case 6:
			cpu_model = 6;
			break;
		default:
			/* What error ?? */
			aerr(SYNTAX_ERROR);
		}
		break;
	/* Implicit one or two byte operation */
	case TIMPL6:
		if (cpu_model <= 4)
			aerr(BADCPU);
	case TIMPL:
		if (opcode > 0x0100)
			outab(opcode >> 8);
		outab(opcode);
		break;
	/* ALU type one register operations. Add 0x10 for word format. If
	   the register is A or AL then add 0x08 and omit the second byte */
	case TREGA8:
		force8 = 1;
	case TREGA:
		getaddr(&a1);
		c = getnb();
		if (c == ',') {
			getaddr(&a2);
			constify(&a2);
			istuser(&a2);
			/* Adjust value. Most ops are ,0 for once. clr however
			   seems to be a straight load of 0-15 */
			if (opcode != 0x22)
				a2.a_value--;
			if (a2.a_value < 0 || a2.a_value > 15)
				aerr(RANGE);
			disp = 1;
			/* The EE200 doesn't have these extensions */
			if (cpu_model <= 4)
				aerr(BADCPU);
		} else {
			disp = 0;
			unget(c);
			a2.a_value = 0;
		}
		switch (a1.a_type & TMMODE) {
		case TWR:
			if (force8)
				aerr(BREGONLY);
			if ((a1.a_type & TMREG)  == RA && !disp)
				outab(opcode | 0x18);
			else {
				outab(opcode | 0x10);
				outab(a2.a_value | ((a1.a_type & TMREG) << 4));
			}
			break;
		case TBR:
			if ((a1.a_type & TMREG)  == RAL && !disp)
				outab(opcode | 0x8);
			else {
				outab(opcode);
				outab(a2.a_value | ((a1.a_type & TMREG) << 4));
			}
			break;
		default:
			aerr(REGONLY);
		}
		break;
	/* As with TREGA but no short form */
	case TREG8:
		force8 = 1;
	case TREG:
		getaddr(&a1);
		c = getnb();
		if (c == ',') {
			getaddr(&a2);
			constify(&a2);
			istuser(&a2);
			if (a2.a_value < 1 || a2.a_value > 16)
				aerr(RANGE);
			/* Adjust value */
			a2.a_value--;
			if (cpu_model <= 4)
				aerr(BADCPU);
		} else {
			unget(c);
			a2.a_value = 0;
		}
		switch (a1.a_type & TMMODE) {
		case TWR:
			if (force8)
				aerr(BREGONLY);
			outab(opcode | 0x10);
			outab(a2.a_value | ((a1.a_type & TMREG) << 4));
			break;
		case TBR:
			outab(opcode);
			outab(a2.a_value | ((a1.a_type & TMREG) << 4));
			break;
		default:
			aerr(REGONLY);
		}
		break;
	/* xfr[b] reg,reg - lots of encodings
		word: 
		word: 55 r1 r2	
		X,A 5B
		Y,A 5C
		B,A 5D
		Z,A 5E
		S,A 5F
		byte:	45 r1 r2
			2E r1 r2	but not A,G
		BL,AL	4D */
		/* TODO PC move */
	case TMOVE8:
		force8 = 1;
	case TMOVE:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		r1 = a1.a_type & TMREG;
		r2 = a2.a_type & TMREG;
		switch(a2.a_type & TMMODE) {
		case TSR:
			if (force8)
				aerr(BREGONLY);
			/* PC */
			if ((a1.a_type & TMMODE) != TWR)
				aerr(WREGONLY);
			if (r2 == RA && r1 == RPC)
				outab(0x0D);
			else
				aerr(BADADDR);
			break;
		case TWR:
			if (force8)
				aerr(BREGONLY);
			if ((a1.a_type & TMMODE) != TWR)
				aerr(WREGONLY);
			if (r2 == RX && r1 == RA)
				outab(0x5B);
			else if (r2 == RY && r1 == RA)
				outab(0x5C);
			else if (r2 == RB && r1 == RA)
				outab(0x5D);
			else if (r2 == RZ && r1 == RA)
				outab(0x5E);
			else if (r2 == RS && r1 == RA)
				outab(0x5F);
			else {
				outab(0x55);
				outab(r2 | (r1 << 4));
			}
			break;
		case TBR:
			if ((a1.a_type & TMMODE) != TBR)
				aerr(BREGONLY);
			if (r1 == RBL && r2 == RAL)
				outab(0x4D);
			else {
				outab(0x45);
				outab(r2 | (r1 << 4));
			}
			break;
		default:
			aerr(REGONLY);
		}
		break;
	case TMMU:
		if (cpu_model <= 4)
			aerr(BADCPU);
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value < 0 || a1.a_value > 7)
			aerr(RANGE);
		comma();
		getaddr(&a2);
		istuser(&a2);
		outab(opcode >> 8);
		outab(opcode);
		outab(a1.a_value | 0xF8);
		outraw(&a2);
		break;
	case TDMA:
		if (cpu_model <= 4)
			aerr(BADCPU);
		/* regpair merged into top of lower byte */
		getaddr(&a1);
		if ((a1.a_type & TMMODE) != TWR) {
			aerr(WREGONLY);
			break;
		}
		outab(opcode >> 8);
		outab(opcode | ((a1.a_type & TMREG) << 4));
		break;
	case TDMAM:
		if (cpu_model <= 4)
			aerr(BADCPU);
		/* DMA mode it's a value not a register encoding */
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value < 0 || a1.a_value > 3) {
			aerr(RANGE);
			break;
		}
		outab(opcode >> 8);
		outab(opcode | (a1.a_value << 4));
		break;
	/* Two register ALU operations with short forms */
	case TREG2A8:
		force8 = 1;
	case TREG2A:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		switch(a1.a_type & TMMODE) {
		case TWR:
			if (force8)
				aerr(BREGONLY);
			if ((a2.a_type & TMMODE) != TWR)
				aerr(WREGONLY);
			if ((a2.a_type & TMREG) == RB &&
				(a1.a_type & TMREG) == RA)
				outab(opcode | 0x18);
			else {
				outab(opcode | 0x10);
				outab((a1.a_type & TMREG) << 4 | (a2.a_type & TMREG));
			}
			break;
		case TBR:
			if ((a2.a_type & TMMODE) != TBR)
				aerr(BREGONLY);
			if ((a2.a_type & TMREG) == RBL &&
				(a1.a_type & TMREG) == RAL)
				outab(opcode | 0x08);
			else {
				outab(opcode);
				outab((a1.a_type & TMREG) << 4 | (a2.a_type & TMREG));
			}
			break;
		default:
			aerr(REGONLY);
		}
		break;		
	/* Two register ALU operations with no word short forms */
	case TREG2ANWS:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		switch(a1.a_type & TMMODE) {
		case TWR:
			if ((a2.a_type & TMMODE) != TWR)
				aerr(WREGONLY);
			outab(opcode | 0x10);
			outab((a2.a_type & TMREG) << 4 | (a1.a_type & TMREG));
			break;
		case TBR:
			if ((a2.a_type & TMMODE) != TBR)
				aerr(BREGONLY);
			if ((a2.a_type & TMREG) == RBL &&
				(a1.a_type & TMREG) == RAL)
				outab(opcode | 0x08);
			else {
				outab(opcode);
				outab((a1.a_type & TMREG) << 4 | (a2.a_type & TMREG));
			}
			break;
		default:
			aerr(REGONLY);
		}
		break;		
	case TJUMP:
		/* Sort of like load/store but the address generated ends up
		   in PC not read from into a register */
		address_encoding(opcode, 2, 0, 2);
		break;
	case TSTORE:
	case TLOAD:
	{
		unsigned encoding = 0x80;
		unsigned store = 0;
		getaddr(&a1);
		if ((sp->s_type & TMMODE) == TSTORE) {
			store = 1;
			encoding |= 0x20;
		}
		comma();
		r1 = a1.a_type & TMREG;
		switch(a1.a_type & TMMODE) {
		case TBR:
			if (r1 == RAL)
				address_encoding(encoding, 0, store, 1);
			else if (r1 == RBL)
				address_encoding(encoding | 0x40, 0, store, 1);
			else
				aerr(REGABBYTE);
			break;	
		case TWR:
			if (force8)
				aerr(BREGONLY);
			encoding |= 0x10;
			if (r1 == RA)
				address_encoding(encoding, 0, store, 2);
			else if (r1 == RB)
				address_encoding(encoding | 0x40, 0, store, 2);
			else if (r1 == RX) {
				/* X is a bit different. Own range and no
				   shorter for register index */
				if (store)
					encoding = 0x68;
				else
					encoding = 0x60;
				address_encoding(encoding, 1, store, 2);
			}
			else
				aerr(REGABXWORD);
			break;
		default:
			aerr(REGONLY);
		}
		break;
	}
	case TLOADEW:
		address_encoding(opcode, 0, 0, 2);
		break;
	case TLOADEB:
		address_encoding(opcode, 0, 0, 1);
		break;
	case TSTOREEW:
		address_encoding(opcode, 0, 1, 2);
		break;
	case TSTOREEB:
		address_encoding(opcode, 0, 1, 1);
		break;
	case TLOADX:
		address_encoding(opcode, 1, 0, 2);
		break;
	case TSTOREX:
		address_encoding(opcode, 1, 1, 2);
		break;

	case TREL8:
		getaddr(&a1);
		/* FIXME: do wo need to check this is constant ? */
		disp = a1.a_value - dot[segment]-2;
		/* Only on pass 3 do we know the correct offset for a forward branch
		   to a label where other stuff with Jcc has been compacted */
		if (pass == 3 && (disp<-128 || disp>127 || a1.a_segment != segment))
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
			if (pass == 0 || disp < -128 || disp > 127 || a1.a_segment != segment)
				c = 1;
			/* On pass 2 we lock down our choices in the table */
			if (pass == 2)
				setnextrel(c);
		}
		/* TODO opcode rework */
		if (c) {
			outab(opcode^1);	/* Inverted branch */
			outab(3);		/* Skip over the jump */
			outab(0x71);		/* Jump */
			outraw(&a1);
		} else {
			outab(opcode);
			/* Should never happen */
			if (disp < -128 || disp > 127)
				aerr(BRA_RANGE);
			outab(disp);
		}
		break;
	case TJUMP8:
		/* Same but not conditional */
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
			if (pass == 0 || disp < -128 || disp > 127 || a1.a_segment != segment)
				c = 1;
			/* On pass 2 we lock down our choices in the table */
			if (pass == 2)
				setnextrel(c);
		}
		if (c) {
			outab(0x71);		/* Jump */
			outraw(&a1);
		} else {
			outab(opcode);
			/* Should never happen */
			if (disp < -128 || disp > 127)
				aerr(BRA_RANGE);
			outab(disp);
		}
		break;
	case TBLOCK:
		if (cpu_model <= 4)
			aerr(BADCPU);
		/* The forms we know */
		getaddr(&a1);
		istuser(&a1);
		c = a1.a_value;
		if (c < 1 || c > 256)
			aerr(RANGE);
		comma();
		getaddr(&a1);
		istuser(&a1);
		comma();
		getaddr(&a2);
		istuser(&a2);
		outab(opcode >> 8);
		outab(opcode);
		outab(c - 1);
		outraw(&a1);
		outraw(&a2);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
