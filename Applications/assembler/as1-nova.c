/*
 * DG Nova assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"

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

	ap->a_sym = NULL;
	ap->a_flags = 0;
	ap->a_type = 0;

	/* We only have one addressing format we ever use.. an address.
	   Quite how we encode it is another saga because our memory ops
	   use register relative or pc relative */
	c = getnb();
	if (c != '#')
		unget(c);
	expr1(ap, LOPRI, 1);
	switch (ap->a_type&TMMODE) {
	case TUSER:
		break;
	default:
		qerr(SYNTAX_ERROR);
	}
	if (c == '@')	/* Indirect */
		ap->a_value |= 0x8000;
}

static int accumulator(void)
{
	int c = getnb();
	if (c < '0' || c > '3') {
		aerr(BAD_ACCUMULATOR);
		unget(c);
		return 0;
	}
	return c - '0';
}

static int carryop(char c)
{
	c = toupper(c);
	if (c == 'Z')
		return 1;
	if (c == 'O')
		return 2;
	if (c == 'C')
		return 3;
	return 0;
}

static int postop(char c)
{
	c = toupper(c);
	if (c == 'L')
		return 1;
	if (c == 'R')
		return 2;
	if (c == 'S')
		return 3;
	return 0;
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
	int acs, acd;
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
	char iid[4];
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
			/* Word machine so half the byte count */
			sp->s_value = dot[segment] >> 1;
			sp->s_segment = segment;
		} else {
			if ((sp->s_type&TMMDF) != 0)
				err('m', MULTIPLE_DEFS);
			if (sp->s_value != dot[segment])
				err('p', PHASE_ERROR);
		}
		goto loop;
	}
	/* We have to do ugly things here because the Nova instruction set
	   merges the opcode and flags */
	memcpy(iid, id, 4);
	iid[3] = 0;
	
	/*
	 * If the first token is an
	 * id and not an operation code,
	 * assume that it is the name in front
	 * of an "equ" assembler directive.
	 */
	if ((sp=lookup(id, phash, 0)) == NULL &&
		(sp = lookup(iid, phash, 0)) == NULL) {
		getid(id1, c);
		if ((sp1=lookup(id1, phash, 0)) == NULL
		||  (sp1->s_type&TMMODE) != TEQU) {
			err('o', SYNTAX_ERROR);
			return;
		}
		getaddr(&a1);
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
		istuser(&a1);
		if (a1.a_segment != ABSOLUTE)
			qerr(MUST_BE_ABSOLUTE);
		outsegment(ABSOLUTE);
		dot[segment] = a1.a_value * 2;	/* dot is in bytes */
		/* Tell the binary generator we've got a new absolute
		   segment. */
		outabsolute(dot[segment]);
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

	case TDEFW:
		do {
			getaddr(&a1);
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
		/* Word machine - pad the end */
		if (dot[segment] & 1)
			dot[segment]++;
		break;

	case TDEFS:
		getaddr(&a1);
		istuser(&a1);
		/* Write out the words. The BSS will deal with the rest */
		for (value = 0 ; value < a1.a_value; value++)
			outaw(0);
		break;

	case TCPUOPT:
		cpu_flags |= sp->s_value;
		break;

	case TMEMORY:
	{
		int indirect = 0;
		/*
		 *	Memory operations are either
		 *	0,disp		Word 0-255 (zero page)
		 *	1,signed disp	PC relative
		 *	2.disp		ac2 + offset
		 *	3,disp		ac3 + offset
		 *
		 *	We don't enforce any rules on ac2/ac3 but encode
		 *	them on the basis the user knows what they are doing
		 *
		 *	0,disp is encoded is an 8 bit relocation for ZP
		 *	1,disp FIXME needs to be encoded as an 8bit PCREL
		 *
		 *	There is *no* immediate load nor is there anyway
		 *	to load arbitrary addresses.
		 *
		 *	FIXME: we need some Nova specific meta ops as a
		 *	result
		 *
		 *	.nomodify	- don't sneak in data words
		 *	.modify		- allowed to sneak in data words
		 *	.local		- local data word
		 *	.flushlocal	- write locals out here
		 *
		 *	local data words are placed in a queue with their
		 *	relocation address. During pass 0 we try to place them
		 *	by adding ,skp to TALU instructions and putting one
		 *	after it. If we reach the point it won't fit we add a
		 *	JMP around the data and load with data. Likewise we
		 *	can fill after a jump.
		 *
		 *	This has its own fun... a jump is itself pcrel or
		 *	constrained. Fortunately however we can encode an
		 *	arbitrary jump as JMP #.+1 and a word. We can't do
		 *	this ourself with JSR but compilers can jmp 3,1
		 *	and do it.
		 *
		 *	For A2/A3 the same way to write stuff is likely to be
		 *
		 *	; a2 is loaded with foo
		 *
		 *	LDA 1,bar-foo,2
		 *
		 *	and the assembler with turn bar-foo into an ABSOLUTE
		 */
		acd = accumulator();
		comma();
		c = get();
		if (c == '@')
			indirect = 1;
		else {
			unget(c);
			indirect = 0;
		}
		getaddr(&a1);
		c = get();
		disp = 0;
		if (c == ',')
			acs = accumulator();
		else
			unget(c);
		/* ,0 means zero page */
		if (acs == 0) {
			if (a1.a_segment == UNKNOWN)
				a1.a_segment = ZP;
			if (a1.a_segment != ZP && a1.a_segment != ABSOLUTE)
				aerr(NEED_ZPABS);
		}
		/* ,2 + are indexes so we can't really police them for sanity */
		/* ,1 is PC relative so must be in this segment */
		if (acs == 1) {
			if (a1.a_segment == UNKNOWN)
				a1.a_segment = segment;
			else if (a1.a_segment != ABSOLUTE && a1.a_segment != segment)
				aerr(BAD_PCREL);
			if (a1.a_segment != ABSOLUTE)
				a1.a_type |= TPCREL;
		}
		/* Insert the accumulators */
		opcode |= (acd << 13);
		opcode |= (acs << 8);
		if (indirect)
			opcode |= 0x0400;
		if (acs)
			outrabrel(&a1);	/* Signed */
		else if (acs == 0)
			outrab(&a1);	/* Unsigned */
		outab(opcode >> 8);
		break;
	}

	case TALU:
	{
		char *p = id + 3;
		SYM *skp = NULL;
		int drop;
		int cf;
		int sh;

		cf = carryop(*p);
		if (cf)
			p++;
		sh = postop(*p);
		if (sh)
			p++;
		c = get();
		if (c == '#')
			drop = 1;
		else {
			unget(c);
			drop = 0;
		}
		acs = accumulator();
		comma();
		acd = accumulator();
		c = getnb();
		if (c == ',') {
			getid(id1,getnb());
			skp = lookup(id1, phash, 0);
			if (skp == NULL || skp->s_type != TCC)
				err('s',SYNTAX_ERROR);
		} else
			unget(c);
		opcode |= (acs << 13);
		opcode |= (acd << 11);
		opcode |= (sh << 6);
		opcode |= (cf << 4);
		opcode |= (drop << 3);
		if (skp)
			opcode |= skp->s_value;
		outaw(opcode);
		break;
	}

	case TIO:
		acs = accumulator();
		comma();
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value > 63)
			err('d', BADDEVICE);
		opcode |= (acs << 1);
		opcode |= a1.a_value;
		outaw(opcode);
		break;

	case TDEV:
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value > 63)
			err('d', BADDEVICE);
		opcode |= a1.a_value;
		outaw(opcode);
		break;

	case TAC:
		acs = accumulator();
		opcode |= (acs << 11);
		outaw(opcode);
		break;

	case TIMPL:
		outaw(opcode);
		break;

	case TBYTE:
		acs = accumulator();
		comma();
		acd = accumulator();
		opcode |= (acd << 11);
		opcode |= (acs << 6);
		outaw(opcode);
		break;

	case TTRAP:
		acs = accumulator();
		comma();		
		acd = accumulator();
		comma();
		getaddr(&a1);
		/* We can't relocate these yet FIXME */
		istuser(&a1);
		opcode |= (acs << 15);
		opcode |= (acd << 13);
		opcode |= (a1.a_value << 4);
		outaw(opcode);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}

