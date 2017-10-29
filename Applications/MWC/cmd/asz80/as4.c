/*
 * Z-80 assembler.
 * Output Intel compatable
 * hex files.
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
		for (i = 1; i < NSEGMENT; i++) {
			segbase[i] = base;
			if (i != BSS) {
				obh.o_segbase[i] = base;
				base += segsize[i] + 2; /* 2 for the EOF mark */
			}
			obh.o_size[i] = truesize[i];
		}
		obh.o_magic = 0;
		obh.o_arch = OA_Z80;
		obh.o_flags = 0;
		/* Will need changing if we add .Z180 and the Z180 ops */
		obh.o_cpuflags = 0;
		obh.o_symbase = base;
		obh.o_dbgbase = 0;	/* for now */
		/* Number the symbols for output */
		numbersymbols();
	}
}

/*
 * Absolute address change
 */

void outabsolute(int addr)
{
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
	outab(w);
	outab(w >> 8);
}

void outraw(ADDR *a)
{
	if (a->a_segment != ABSOLUTE) {
		if (segment == BSS)
			err('b', DATA_IN_BSS);
		if (a->a_sym == NULL) {
			outbyte(REL_ESC);
			outbyte((1 << 4) | REL_SIMPLE | a->a_segment);
		} else {
			outbyte(REL_ESC);
			outbyte((1 << 4 ) | REL_SYMBOL);
			outbyte(a->a_sym->s_number & 0xFF);
			outbyte(a->a_sym->s_number >> 8);
		}
	}
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
	if (segment == BSS && b)
		err('b', DATA_IN_BSS);
	if (segment == ABSOLUTE)
		err('A', MUST_BE_ABSOLUTE);
	outbyte(b);
	if (b == REL_ESC)	/* Quote relocation markers */
		outbyte(REL_REL);
	++dot[segment];
	++truesize[segment];
	if (truesize[segment] == 0 || dot[segment] == 0)
		err('o', SEGMENT_OVERFLOW);
}

void outrab(ADDR *a)
{
	/* FIXME: handle symbols */
	if (a->a_segment != ABSOLUTE) {
		if (segment == BSS)
			err('b', DATA_IN_BSS);
		if (a->a_sym == NULL) {
			outbyte(REL_ESC);
			outbyte((0 << 4) | REL_SIMPLE | a->a_segment);
		} else {
			outbyte(REL_ESC);
			outbyte((0 << 4 ) | REL_SYMBOL);
			outbyte(a->a_sym->s_number & 0xFF);
			outbyte(a->a_sym->s_number >> 8);
		}
	}
	outab(a->a_value);
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
	/* 0 absolute, 1-n segments, -1 don't care */
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
/*	printf("Code %d bytes: Data %d bytes: BSS %d bytes\n",
		truesize[CODE], truesize[DATA], truesize[BSS]); */
}

/*
 * Output a byte and track our position. For BSS we care about sizes
 * only.
 */
void outbyte(uint8_t b)
{
	if (pass == 1 && segment != BSS)
		putc(b, ofp);
	segbase[segment]++;
	segsize[segment]++;
}
