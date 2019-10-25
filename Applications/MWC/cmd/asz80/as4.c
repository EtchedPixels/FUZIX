/*
 * Output FUZIX object files for 8/16bit machines.
 *
 * Currently we understand little endian 8 and 16bit formats along with
 * PC relative.
 *
 * FIXME: We need to manage dot[segment] properly for word addressed machines
 * so that we track and write in words not bytes. Right now word addressing
 * is broken
 */

#include	"as.h"

static uint16_t segsize[NSEGMENT];
static uint16_t truesize[NSEGMENT];
static off_t segbase[NSEGMENT];

static struct objhdr obh;

static void numbersymbols(void);

void outpass(void)
{
	off_t base = sizeof(obh);
	int i;

	if (pass == 1) {
		/* Lay the file out */
		for (i = 0; i < NSEGMENT; i++) {
			segbase[i] = base;
			if (i != BSS && i != ZP) {
				obh.o_segbase[i] = base;
				base += segsize[i] + 2; /* 2 for the EOF mark */
			}
			obh.o_size[i] = truesize[i];
		}
		obh.o_magic = 0;
		obh.o_arch = ARCH;
		obh.o_flags = ARCH_FLAGS;
		obh.o_cpuflags = cpu_flags;
		obh.o_symbase = base;
		obh.o_dbgbase = 0;	/* for now */
		/* Number the symbols for output */
		numbersymbols();
		segment = ABSOLUTE;
		outsegment(segment);
	}
}

/*
 * Absolute address change
 */

void outabsolute(int addr)
{
	if (segment != ABSOLUTE)
		qerr(MUST_BE_ABSOLUTE);
	else {
		outbyte(REL_ESC);
		outbyte(REL_ORG);
		outbyte(addr & 0xFF);
		outbyte(addr >> 8);
	}
}

/*
 * Segment change
 */

void outsegment(int seg)
{
	/* Seek to the current writing address for this segment */
	if (pass == 1)
		fseek(ofp, segbase[seg], SEEK_SET);
}

/*
 * Output a word. Use the
 * standard Z-80 ordering (low
 * byte then high byte).
 */
void outaw(uint16_t w)
{
#ifdef TARGET_BE
	outab(w >> 8);
	outab(w);
#else
	outab(w);
	outab(w >> 8);
#endif
}

static void check_store_allowed(uint8_t segment, uint16_t value)
{
	if (value == 0)
		return;
	if (segment == BSS)
		err('b', DATA_IN_BSS);
	if (segment == ZP)
		err('z', DATA_IN_ZP);
}

/*
 *	Symbol numbers and relocations are always written little endian
 *	for simplicity.
 *
 *	A_LOW and A_HIGH indicate 8bit partial relocations. We handle these
 *	internally.
 */
void outraw(ADDR *a)
{
	int s = 1 << 4;
	/* We must insert a relocation record for anything relocatable,
	   but also for anything which is a symbol, as the linker may
	   need to do absolute resolution between modules */
	if (a->a_segment != ABSOLUTE || a->a_sym) {
		outbyte(REL_ESC);
		check_store_allowed(segment, 1);
		/* low bits of 16 bit is an 8bit relocation with
		   overflow suppressed */
		if (a->a_flags & A_LOW) {
			outbyte(REL_OVERFLOW);
			s = 0 << 4;
		}
		if (a->a_flags & A_HIGH)
			outbyte(REL_HIGH);
		if (a->a_sym == NULL) {
			/* low bits of 16 bit is an 8bit relocation with
			   overflow suppressed */
			outbyte(s | REL_SIMPLE | a->a_segment);
		} else {
			outbyte(s | REL_SYMBOL);
			outbyte(a->a_sym->s_number & 0xFF);
			outbyte(a->a_sym->s_number >> 8);
		}
	}
	if (a->a_flags & A_LOW)
		outab(a->a_value);
	else
		outaw(a->a_value);
}

/*
 * Output an absolute
 * byte to the code and listing
 * streams.
 */
void outab(uint8_t b)
{
	/* Not allowed to put data in the BSS except zero */
	check_store_allowed(segment, b);
	outbyte(b);
	if (b == REL_ESC)	/* Quote relocation markers */
		outbyte(REL_REL);
	++dot[segment];
	++truesize[segment];
	if (truesize[segment] == SEGMENT_LIMIT || dot[segment] == SEGMENT_LIMIT)
		err('o', SEGMENT_OVERFLOW);
}

void outabchk(uint16_t b)
{
	if (b > 255)
		err('o', CONSTANT_RANGE);
	outab(b);
}

void outrabrel(ADDR *a)
{
	check_store_allowed(segment, 1);
	if (a->a_sym) {
		outbyte(REL_ESC);
		outbyte((0 << 4 ) | REL_PCREL);
		outbyte(a->a_sym->s_number & 0xFF);
		outbyte(a->a_sym->s_number >> 8);
		outbyte(a->a_value);
		outab(a->a_value >> 8);
		return;
	}
	/* relatives without a symbol don't need relocation */
	if (a->a_value < -128 || a->a_value > 127)
		err('o', CONSTANT_RANGE);
	outab(a->a_value);
}

/*
 *	We should probably fold outraw and outrab as they are not very
 *	different except in the default value of s.
 */
void outrab(ADDR *a)
{
	int s = 0;
	uint16_t mode = (a->a_type & TMMODE);
	if (mode == TBR || mode == TWR) {
		a->a_flags |= A_LOW;
		if (a->a_segment != ABSOLUTE)
			aerr(MUST_BE_ABSOLUTE);
	}
	if (a->a_segment != ABSOLUTE) {
		check_store_allowed(segment, 1);
		outbyte(REL_ESC);
		if (a->a_flags & A_LOW) {
			outbyte(REL_OVERFLOW);
			a->a_value &= 0xFF;
		}
		if (a->a_flags & A_HIGH) {
			outbyte(REL_HIGH);
			s = 1 << 4;
		}
		if (a->a_sym == NULL) {
			outbyte(s | REL_SIMPLE | a->a_segment);
		} else {
			outbyte(s | REL_SYMBOL);
			outbyte(a->a_sym->s_number & 0xFF);
			outbyte(a->a_sym->s_number >> 8);
		}
	}
	if (s)
		outaw(a->a_value);
	else
		outabchk(a->a_value);
}

static void putsymbol(SYM *s, FILE *ofp)
{
	int i;
	uint8_t flag = 0;
	if (s->s_type == TNEW)
		flag |= S_UNKNOWN;
	else {
		if (s->s_type & TPUBLIC)
			flag |= S_PUBLIC;
	}
	/* 0 absolute, 1-n segments, 15 don't care */
	flag |= (s->s_segment & S_SEGMENT);
	putc(flag, ofp);
	fwrite(s->s_id, 16, 1, ofp);
	putc(s->s_value, ofp);
	putc(s->s_value >> 8, ofp);
}

static void enumerate(SYM *s, FILE *dummy)
{
	static int sym = 0;
	s->s_number = sym++;
}

static void dosymbols(SYM *hash[], FILE *ofp, int flag, void (*op)(SYM *, FILE *f))
{
	int i;
	for (i = 0; i < NHASH; i++) {
		SYM *s;
		for (s = hash[i]; s != NULL; s = s->s_fp) {
			int t = s->s_type & TMMODE;
			int n;
			if (t != TUSER && t != TNEW)
				continue;
			n =  (t == TNEW) || (t == TUSER && (s->s_type & TPUBLIC));
			if (n == flag)
				op(s, ofp);
		}
	}
}

static void writesymbols(SYM *hash[], FILE *ofp)
{
	fseek(ofp, obh.o_symbase, SEEK_SET);
	dosymbols(hash, ofp, 1, putsymbol);
	obh.o_dbgbase = ftell(ofp);
	if (debug_write) {
		dosymbols(uhash, ofp, 0, putsymbol);
	}
}

static void numbersymbols(void)
{
	dosymbols(uhash, NULL, 1, enumerate);
}

/*
 * Put out the end of file
 * hex item at the very end of
 * the object file.
 */
void outeof(void)
{
	/* We don't do the final write out if there was an error. That
	   leaves the magic wrong on the object file so it can't be used */
	if (noobj || pass == 0)
		return;

	segment = ABSOLUTE;
	outsegment(ABSOLUTE);
	outbyte(REL_ESC);
	outbyte(REL_EOF);
	segment = CODE;
	outsegment(CODE);
	outbyte(REL_ESC);
	outbyte(REL_EOF);
	segment = DATA;
	outsegment(DATA);
	outbyte(REL_ESC);
	outbyte(REL_EOF);
	writesymbols(uhash, ofp);
	rewind(ofp);
	obh.o_magic = MAGIC_OBJ;
	fwrite(&obh, sizeof(obh), 1, ofp);
//	printf("Abs %d bytes: Code %d bytes: Data %d bytes: BSS %d bytes\n",
//		truesize[ABSOLUTE], truesize[CODE], truesize[DATA], truesize[BSS]);
}

/*
 * Output a byte and track our position. For BSS we care about sizes
 * only. ZP is similar but not yet really used.
 */
void outbyte(uint8_t b)
{
	if (pass == 1 && segment != BSS && segment != ZP)
		putc(b, ofp);
	segbase[segment]++;
	segsize[segment]++;
}
