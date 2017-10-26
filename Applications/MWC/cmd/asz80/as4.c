/*
 * Z-80 assembler.
 * Output Intel compatable
 * hex files.
 */

#include	"as.h"
#include	"obj.h"

#define	NHEX	8			/* Nice format size */

static uint16_t segsize[NSEGMENT];
static uint16_t truesize[NSEGMENT];
static off_t segbase[NSEGMENT];

struct objhdr obh;

static void outc(char c);

void outpass(void)
{
	off_t base = sizeof(obh);
	int i;
	if (pass == 1) {
		/* Lay the file out */
		for (i = 1; i < NSEGMENT; i++) {
			if (i != BSS) {
				obh.o_segbase[i] = base;
				segbase[i] = base;
				printf("BASE %d %d\n", i, base);
				base += segsize[i];
				printf("SIZE %d %d\n", i, truesize[i]);
			}
			obh.o_size[i] = truesize[i];
		}
		obh.o_magic = 0;
		obh.o_symbase = base;
		obh.o_dbgbase = 0;	/* for now */
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
		/* FIXME@ handle symbols */
		if (segment == BSS && a->a_value)
			err('b');
		outbyte(REL_ESC);
		outbyte((2 << 4) | REL_SIMPLE | a->a_segment);
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
		err('b');
	if (segment == ABSOLUTE)
		err('A');
	outbyte(b);
	if (b == 0xDA)	/* Quote relocation markers */
		outbyte(0x00);
	++dot[segment];
	++truesize[segment];
	if (truesize[segment] == 0 || dot[segment] == 0)
		err('o');
}

void outrab(ADDR *a)
{
	/* FIXME: handle symbols */
	if (segment == BSS && a->a_value) {
		err('b');
		outbyte(REL_ESC);
		outbyte((2 << 4) | REL_SIMPLE | a->a_segment);
	}
	outab(a->a_value);
}

/*
 * Put out the end of file
 * hex item at the very end of
 * the object file.
 */
void outeof(void)
{
	rewind(ofp);
	obh.o_magic = MAGIC_OBJ;
	fwrite(&obh, sizeof(obh), 1, ofp);
	printf("Code %d byyes: Data %d bytes: BSS %d bytes\n",
		truesize[CODE], truesize[DATA], truesize[BSS]);
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
