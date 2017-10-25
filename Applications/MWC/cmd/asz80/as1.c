/*
 * Z-80 assembler.
 * Assemble one line of input.
 * Knows all the dirt.
 */
#include	"as.h"

#define	OPDJNZ	0x10			/* Opcode: djnz */
#define	OPADD	0x80			/* Opcode: add */
#define	OPDAD	0x09			/* Opcode: dad */
#define	OPADC	0x88			/* Opcode: adc */
#define	OPADCW	0x4A			/* Opcode: adc hl */
#define	OPSBCW	0x42			/* Opcode: sbc hl */
#define	OPSUBI	0xC6			/* Opcode: make immediate */
#define	OPXCHG	0xEB			/* Opcode: xchg */
#define	OPXTHL	0xE3			/* Opcode: xthl */
#define	OPEXAF	0x08			/* Opcode: ex af,af' */
#define	OPRST	0xC7			/* Opcode: rst 0 */
#define	OPINCRP	0x03			/* Opcode: inc rp */
#define	OPDECRP	0x0B			/* Opcode: dec rp */
#define	OPINC	0x04			/* Opcode: inc */
#define	OPIIN	0x40			/* Opcode: indirect in */
#define	OPIOUT	0x41			/* Opcode: indirect out */
#define	OPIN	0xDB			/* Opcode: in */
#define	OPIM	0x46			/* Opcode: im */
#define	OPPCHL	0xE9			/* Opcode: jp (hl) */
#define	OPJP	0xC3			/* Opcode: jp cc base */
#define	OPJR	0x20			/* Opcode: jr cc base */
#define	OPRET	0xC0			/* Opcode: ret cc base */

/*
 * Assemble one line.
 * The line in in "ib", the "ip"
 * scans along it. The code is written
 * right out, and also stashed in the
 * "cb" for the listing.
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

	laddr = dot;
	lmode = SLIST;
loop:
	if ((c=getnb())=='\n' || c==';')
		return;
	if (isalpha(c) == 0)
		qerr();
	getid(id, c);
	if ((c=getnb()) == ':') {
		sp = lookup(id, uhash, 1);
		if (pass == 0) {
			if ((sp->s_type&TMMODE) != TNEW
			&&  (sp->s_type&TMASG) == 0)
				sp->s_type |= TMMDF;
			sp->s_type &= ~TMMODE;
			sp->s_type |= TUSER;
			sp->s_value = dot;
		} else {
			if ((sp->s_type&TMMDF) != 0)
				err('m');
			if (sp->s_value != dot)
				err('p');
		}
		lmode = ALIST;
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
			err('o');
			return;
		}
		getaddr(&a1);
		istuser(&a1);
		sp = lookup(id, uhash, 1);
		if ((sp->s_type&TMMODE) != TNEW
		&&  (sp->s_type&TMASG) == 0)
			err('m');
		sp->s_type &= ~TMMODE;
		sp->s_type |= TUSER|TMASG;
		sp->s_value = a1.a_value;
		laddr = a1.a_value;
		lmode = ALIST;
		goto loop;
	}
	unget(c);
	lmode = BLIST;
	opcode = sp->s_value;
	switch (sp->s_type&TMMODE) {
	case TORG:
		getaddr(&a1);
		istuser(&a1);
		lmode = ALIST;
		laddr = dot = a1.a_value;
		break;

	case TDEFB:
		do {
			getaddr(&a1);
			istuser(&a1);
			outab(a1.a_value);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFW:
		lmode = WLIST;
		do {
			getaddr(&a1);
			istuser(&a1);
			outaw(a1.a_value);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFM:
		if ((delim=getnb()) == '\n')
			qerr();
		while ((c=get()) != delim) {
			if (c == '\n')
				qerr();
			outab(c);
		}
		break;

	case TDEFS:
		laddr = dot;
		lmode = ALIST;
		getaddr(&a1);
		istuser(&a1);
		dot += a1.a_value;
		break;

	case TNOP:
		if ((opcode&0xFF00) != 0)
			outab(opcode >> 8);
		outab(opcode);
		break;

	case TRST:
		getaddr(&a1);
		istuser(&a1);
		if (a1.a_value < 8) {
			outab(OPRST|(a1.a_value<<3));
			break;	
		}
		aerr();
		break;

	case TREL:
		getaddr(&a1);
		if ((cc=ccfetch(&a1)) >= 0) {
			if (opcode==OPDJNZ || cc>=CPO)
				aerr();
			opcode = OPJR | (cc<<3);
			comma();
			getaddr(&a1);
		}
		istuser(&a1);
		disp = a1.a_value-dot-2;
		if (disp<-128 || disp>127)
			aerr();
		outab(opcode);
		outab(disp);
		break;

	case TRET:
		unget(c = getnb());
		if (c!='\n' && c!=';') {
			getaddr(&a1);
			if ((cc=ccfetch(&a1)) < 0)
				aerr();
			opcode = OPRET | (cc<<3);
		}
		outab(opcode);
		break;

	case TJMP:
		getaddr(&a1);
		if ((cc=ccfetch(&a1)) >= 0) {
			opcode = (opcode&0x00C6) | (cc<<3);
			comma();
			getaddr(&a1);
		}
		if ((a1.a_type&TMMODE) == TBR) {
			reg = a1.a_type&TMREG;
			if (reg==M || reg==IX || reg==IY) {
				if (opcode != OPJP)
					aerr();
				outop(OPPCHL, &a1);
				break;
			}
		}
		istuser(&a1);
		outab(opcode);
		outaw(a1.a_value);
		break;

	case TPUSH:
		getaddr(&a1);
		if ((a1.a_type&TMMODE) == TWR) {
			reg = a1.a_type&TMREG;
			switch (reg) {
			case IX:
				outab(0xDD);
				reg = HL;
				break;
			case IY:
				outab(0xFD);
				reg = HL;
				break;
			case AF:
				reg = SP;
				break;
			case SP:
			case AFPRIME:
				aerr();
			}
			outab(opcode|(reg<<4));
			break;
		}
		aerr();
		break;

	case TIM:
		getaddr(&a1);
		istuser(&a1);
		if ((value=a1.a_value) > 2)
			aerr();
		else if (value != 0)
			++value;
		outab(0xED);
		outab(OPIM|(value<<3));
		break;

	case TIO:
		getaddr(opcode==OPIN ? &a1 : &a2);
		comma();
		getaddr(opcode==OPIN ? &a2 : &a1);
		if (a1.a_type==(TBR|A) && a2.a_type==(TUSER|TMINDIR)) {
			outab(opcode);
			outab(a2.a_value);
			break;
		}
		if ((a1.a_type&TMMODE)==TBR && a2.a_type==(TBR|TMINDIR|C)) {
			reg = a1.a_type&TMREG;
			if (reg==M || reg==IX || reg==IY)
				aerr();
			outab(0xED);
			if (opcode == OPIN)
				opcode = OPIIN; else
				opcode = OPIOUT;
			outab(opcode|(reg<<3));
			break;
		}
		aerr();
		break;

	case TBIT:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if ((a1.a_type&TMMODE) == TUSER
		&&   a1.a_value < 8
		&&  (a2.a_type&TMMODE) == TBR) {
			if ((reg=a2.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|(a1.a_value<<3)|reg, &a2);
			break;
		}
		aerr();
		break;

	case TSHR:
		getaddr(&a1);
		if ((a1.a_type&TMMODE) == TBR) {
			if ((reg=a1.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|reg, &a1);
			break;
		}
		aerr();

	case TINC:
		getaddr(&a1);
		if ((a1.a_type&TMMODE) == TWR) {
			reg = a1.a_type&TMREG;
			switch (reg) {
			case IX:
				outab(0xDD);
				reg = HL;
				break;
			case IY:
				outab(0xFD);
				reg = HL;
				break;
			case AF:
			case AFPRIME:
				aerr();
			}
			if (opcode == OPINC)
				opcode = OPINCRP; else
				opcode = OPDECRP;
			outab(opcode|(reg<<4));
			break;
		}
		if ((a1.a_type&TMMODE) == TBR) {
			if ((reg=a1.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|(reg<<3), &a1);
			break;
		}
		aerr();
		break;

	case TEX:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if (a1.a_type==(TWR|AF) && a2.a_type==(TWR|AFPRIME)) {
			outab(OPEXAF);
			break;
		}
		if (a1.a_type == (TWR|DE))
			opcode = OPXCHG;
		else if (a1.a_type == (TWR|TMINDIR|SP))
			opcode = OPXTHL;
		else
			aerr();
		if (a2.a_type == (TWR|HL))
			outab(opcode);
		else if (a2.a_type == (TWR|IX)) {
			outab(0xDD);
			outab(opcode);
		} else if (a2.a_type == (TWR|IY)) {
			outab(0xFD);
			outab(opcode);
		} else
			aerr();
		break;

	case TSUB:
		getaddr(&a1);
		if (a1.a_type == TUSER) {
			outab(opcode | OPSUBI);
			outab(a1.a_value);
			break;
		}
		if ((a1.a_type&TMMODE) == TBR) {
			if ((reg=a1.a_type&TMREG)==IX || reg==IY)
				reg = M;
			outop(opcode|reg, &a1);
			break;
		}
		aerr();
		break;

	case TADD:
		getaddr(&a1);
		comma();
		getaddr(&a2);
		if (a1.a_type == (TBR|A)) {
			if (a2.a_type == TUSER) {
				outab(opcode | OPSUBI);
				outab(a2.a_value);
				break;
			}
			if ((a2.a_type&TMMODE) == TBR) {
				if ((reg=a2.a_type&TMREG)==IX || reg==IY)
					reg = M;
				outop(opcode|reg, &a1);
				break;
			}
		}
		if ((a1.a_type&TMMODE) == TWR) {
			switch(reg = a1.a_type&TMREG) {
			case IX:
				if (opcode != OPADD)
					aerr();
				outab(0xDD);
				opcode = OPDAD;
				srcreg = IX;
				break;
			case IY:
				if (opcode != OPADD)
					aerr();
				outab(0xFD);
				opcode = OPDAD;
				srcreg = IY;
				break;
			case HL:
				if (opcode == OPADD)
					opcode = OPDAD;
				else {
					outab(0xED);
					if (opcode == OPADC)
						opcode = OPADCW;
					else
						opcode = OPSBCW;
				}
				srcreg = HL;
				break;
			default:
				aerr();
			}
			if ((a2.a_type&TMMODE) == TWR) {
				reg = a2.a_type&TMREG;
				if (reg==BC || reg==DE || reg==SP) {
					outab(opcode|(reg<<4));
					break;
				}
				if (reg == srcreg) {
					outab(opcode|(HL<<4));
					break;
				}
			}
		}
		aerr();
		break;

	case TLD:
		asmld();
		break;

	default:
		err('o');
	}
	goto loop;
}

/*
 * Handle the dreaded "ld" instruction,
 * with its dozens of forms, formats and special
 * encodings. The "getldaddr" routine performs most
 * of the special stuff for index registers and for
 * indexing. This layer just screens out the many
 * cases, and emits the correct bytes.
 */
void asmld(void)
{
	int mdst;
	int rdst;
	int msrc;
	int rsrc;
	ADDR *indexap;
	ADDR dst;
	ADDR src;

	indexap = NULL;
	indexap = getldaddr(&dst, &mdst, &rdst, indexap);
	comma();
	indexap = getldaddr(&src, &msrc, &rsrc, indexap);
	if (dst.a_type == (TBR|A)) {
		if (msrc == TSR) {
			if (rsrc == I)
				outaw(0x57ED);		/* ld a,i */
			else
				outaw(0x5FED);		/* ld a,r */
			return;
		}
		if (msrc == (TMINDIR|TUSER)) {
			outab(0x3A);			/* lda */
			outaw(src.a_value);
			return;
		}
		if (msrc == (TMINDIR|TWR)) {
			if (rsrc==BC || rsrc==DE) {
				outab(0x0A|(rsrc<<4));	/* ldax */
				return;
			}
		}
	}
	if (src.a_type == (TBR|A)) {
		if (mdst == TSR) {
			if (rdst == I)
				outaw(0x47ED);		/* ld i,a */
			else
				outaw(0x4FED);		/* ld r,a */
			return;
		}
		if (mdst == (TMINDIR|TUSER)) {
			outab(0x32);			/* sta */
			outaw(dst.a_value);
			return;
		}
		if (mdst == (TMINDIR|TWR)) {
			if (rdst==BC || rdst==DE) {
				outab(0x02|(rdst<<4));	/* stax */
				return;
			}
		}
	}
	if (dst.a_type==(TWR|SP) && msrc==TWR) {
		if (rsrc == HL) {
			outab(0xF9);			/* sphl */
			return;
		}
	}
	if (msrc == TUSER) {
		if (mdst == TBR) {
			outab(0x06|(rdst<<3));		/* mvi */
			if (indexap != NULL)
				outab(indexap->a_value);
			outab(src.a_value);
			return;
		}
		if (mdst == TWR) {
			outab(0x01|(rdst<<4));		/* lxi */
			outaw(src.a_value);
			return;
		}
	}
	if (mdst==TWR && msrc==(TMINDIR|TUSER)) {
		if (rdst == HL)
			outab(0x2A);			/* lhld */
		else
			outaw(0x4BED|(rdst<<12));	/* ld rp,(ppqq) */
		outaw(src.a_value);
		return;
	}
	if (mdst==(TMINDIR|TUSER) && msrc==TWR) {
		if (rsrc == HL)
			outab(0x22);			/* shld */
		else
			outaw(0x43ED|(rsrc<<12));	/* ld (ppqq),rp */
		outaw(dst.a_value);
		return;
	}
	if (mdst==TBR && msrc==TBR && (rdst!=M || rsrc!=M)) {
		outab(0x40|(rdst<<3)|rsrc);
		if (indexap != NULL)
			outab(indexap->a_value);
		return;
	}
	aerr();
}

/*
 * Read in addresses for "ld"
 * instructions. Split off the mode
 * and the register name. Adjust the register
 * name to correctly deal with the index registers
 * and for indexed addressing modes. Return the address
 * pointer "ap" if indexing is required, otherwise just
 * pass the "iap" through.
 */
ADDR	*getldaddr(ADDR *ap, int *modep, int *regp, ADDR *iap)
{
	int mode;
	int reg;

	getaddr(ap);
	mode = ap->a_type&TMMODE;
	reg  = ap->a_type&TMREG;
	switch (ap->a_type) {
	case TBR|IX:
		outab(0xDD);
		reg = M;
		iap = ap;
		break;

	case TBR|IY:
		outab(0xFD);
		reg = M;
		iap = ap;
		break;

	case TWR|IX:
		outab(0xDD);
		reg = HL;
		break;

	case TWR|IY:
		outab(0xFD);
		reg = HL;
		break;

	case TWR|AF:
	case TWR|AFPRIME:
		aerr();
		reg = HL;
	}
	*modep = mode;
	*regp  = reg;
	return (iap);
}

/*
 * Output an opcode, surrounded
 * by the index prefix bytes and the
 * index displacement byte. Look at
 * the address mode to see if the bytes
 * are needed.
 */
void outop(int op, ADDR *ap)
{
	int needisp;

	needisp = 0;
	if (ap->a_type == (TBR|IX)) {
		outab(0xDD);
		needisp = 1;
	} else if (ap->a_type == (TBR|IY)) {
		outab(0xFD);
		needisp = 1;
	}
	if ((op&0xFF00) != 0) {
		outab(op>>8);
		if (needisp != 0) {
			outab(ap->a_value);
			needisp = 0;
		}
	}
	outab(op);
	if (needisp != 0)
		outab(ap->a_value);
}

/*
 * The next character
 * in the input must be a comma
 * or it is a fatal error.
 */
void comma(void)
{
	if (getnb() != ',')
		qerr();
}

/*
 * Check if the mode of
 * an ADDR is TUSER. If not, give
 * an error.
 */
void istuser(ADDR *ap)
{
	if ((ap->a_type&TMMODE) != TUSER)
		aerr();
}

/*
 * Try to interpret an "ADDR"
 * as a condition code name. Return
 * the condition, or "-1" if it cannot
 * be interpreted as a condition. The
 * "c" condition is a pain.
 */
int ccfetch(ADDR *ap)
{
	if (ap->a_type == (TBR|C))
		return (CC);
	if ((ap->a_type&TMMODE) == TCC)
		return (ap->a_type&TMREG);
	return (-1);
}
