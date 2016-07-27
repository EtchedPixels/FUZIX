
/* writefuzix.c - write binary file for linker */

/* Based upon writebin Copyright (C) 1994 Bruce Evans */

/*
 * TODO
 * - add support for debug symbol tables
 * - figure out how banked binaries should look
 * - add a discard segment that is put between data and bss and the
 *   bss size shrunk by its size (so the bootstrap code jumps to discard,
 *   relocates and then it gets wiped)
 */

#include "syshead.h"
#include "bindef.h"
#include "const.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#define btextoffset (text_base_value)
#define bdataoffset (data_base_value)

struct fuzix16_hdr {
    uint8_t jmp;
    uint16_t addr;
    uint8_t magic[4];
    uint8_t load;
    uint16_t chmem;
    uint16_t code;
    uint16_t data;
    uint16_t bss;
    uint16_t mbz;
    /* Header can be extended as the jump goes over it */
};
    
#define HEADERSIZE sizeof(struct fuzix16_hdr)
/* It's part of the binary */
#define FILEHEADERLENGTH	0
/* Must match AS09 */
#define DPSEG 3

#define CM_MASK 0xC0
#define MODIFY_MASK 0x3F
#define S_MASK 0x04
#define OF_MASK 0x03

#define CM_SPECIAL 0
#define CM_ABSOLUTE 0x40
#define CM_OFFSET_RELOC 0x80
#define CM_SYMBOL_RELOC 0xC0

#define CM_EOT 0
#define CM_BYTE_SIZE 1
#define CM_WORD_SIZE 2
#define CM_LONG_SIZE 3
#define CM_1_SKIP 17
#define CM_2_SKIP 18
#define CM_4_SKIP 19
#define CM_0_SEG 32

#define ABS_TEXT_MAX 64

#define memsizeof(struc, mem) sizeof(((struc *) 0)->mem)

static bool_t bits32;		/* nonzero for 32-bit executable */
static bin_off_t combase[NSEG];/* bases of common parts of segments */
static bin_off_t comsz[NSEG];	/* sizes of common parts of segments */
static fastin_t curseg;	/* current segment, 0 to $F */
static bin_off_t edataoffset;	/* end of data */
static bin_off_t endoffset;	/* end of bss */
static bin_off_t etextoffset;	/* end of text */
static bin_off_t etextpadoff;	/* end of padded text */
static unsigned relocsize;	/* current relocation size 1, 2 or 4 */
static bin_off_t segadj[NSEG];	/* adjusts (file offset - seg offset) */
				/* depends on zero init */
static bin_off_t segbase[NSEG];/* bases of data parts of segments */
static char segboundary[9] = "__seg0DH";
				/* name of seg boundary __seg0DL to __segfCH */
static bin_off_t segpos[NSEG];	/* segment positions for current module */
static bin_off_t segsz[NSEG];	/* sizes of data parts of segments */
				/* depends on zero init */
static bool_t stripflag;	/* nonzero to strip symbols */
static bin_off_t spos;		/* position in current seg */
static bool_t uzp;		/* nonzero for unmapped zero page */
static bool_t sepid;		/* Separate I & D space (8086 etc) */

static void linkmod(struct modstruct *modptr);
static void padmod(struct modstruct *modptr);
static void setsym(char *name, bin_off_t value);
static void symres(char *name);
static void setseg(fastin_pt newseg);
static void skip(unsigned countsize);
static void writeheader(void);
static void writenulls(bin_off_t count);

void write_fuzix(char *outfilename, bool_pt argsepid, bool_pt argbits32,
                     bool_pt argstripflag, bool_pt arguzp)
{
    char *cptr;
    struct modstruct *modptr;
    fastin_t seg;
    unsigned sizecount;
    bin_off_t tempoffset;

    sepid = argsepid;
    bits32 = argbits32;
    stripflag = argstripflag;
    uzp = arguzp;

    /* reserve special symbols use curseg to pass parameter to symres() */
    for (curseg = 0; curseg < NSEG; ++curseg)
    {
	segboundary[5] = hexdigit[curseg];	/* to __segX?H */
	segboundary[6] = 'D';
	symres(segboundary);	/* __segXDH */
	segboundary[7] = 'L';
	symres(segboundary);	/* __segXDL */
	segboundary[6] = 'C';
	symres(segboundary);	/* __segXCL */
	segboundary[7] = 'H';
	symres(segboundary);	/* __segXCH */
#ifndef DATASEGS
        if( curseg > 3 )
	{
	   segboundary[6] = 'S';
	   segboundary[7] = 'O';
	   symres(segboundary); /* __segXSO */
        }
#endif
    }
    /* FIXME: we need a BSS segment in here ??? */
    curseg = 3;
    symres("__edata");
    symres("__end");
    curseg = 0;			/* text seg, s.b. variable */
    symres("__etext");
    symres("__segoff");

    /* calculate segment and common sizes (sum over loaded modules) */
    /* use zero init of segsz[] */
    /* also relocate symbols relative to starts of their segments */
    for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    register struct symstruct **symparray;
	    register struct symstruct *symptr;

	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != NUL_PTR; ++symparray)
		if (symptr->modptr == modptr && !(symptr->flags & A_MASK))
		{
		    if (!(symptr->flags & (I_MASK | SA_MASK)))
		    {
			/* relocate by offset of module in segment later */
			/* relocate by offset of segment in memory special */
			/* symbols get relocated improperly */
			symptr->value += segsz[symptr->flags & SEGM_MASK];
		    }
		    else if (symptr->value == 0)
			    undefined(symptr->name);
		    else
		    {
			tempoffset = ld_roundup(symptr->value, 4, bin_off_t);
			/* temp kludge quad alignment for 386 */
			symptr->value = comsz[seg = symptr->flags & SEGM_MASK];
			comsz[seg] += tempoffset;
			if (!(symptr->flags & SA_MASK))
			    symptr->flags |= C_MASK;
		    }
		}
	    for (seg = 0, cptr = modptr->segsize; seg < NSEG; ++seg)
	    {
		segsz[seg] += cntooffset(cptr,
			  sizecount = segsizecount((unsigned) seg, modptr));
		cptr += sizecount;
	    }
	}

    /* calculate seg positions now their sizes are known */
    /* temp use fixed order 0D 0C 1D 1C 2D 2C ... */
    /*
#ifdef DATASEGS
     * Assume seg 0 is text and rest are data
#else
     * Assume seg 1..3 are data, Seg 0 is real text, seg 4+ are far text
#endif
     */
    segpos[0] = segbase[0] = spos = btextoffset;
    combase[0] = segbase[0] + segsz[0];
    segadj[1] = segadj[0] = -btextoffset;
    etextpadoff = etextoffset = combase[0] + comsz[0];
    /* No rounding needed currently - but keep for x86 etc when we get
       to them */
#if 0    
    if (sepid)
    {
	etextpadoff = ld_roundup(etextoffset, 0x10, bin_off_t);
	segadj[1] += etextpadoff - bdataoffset;
    } else
#endif    
    if (bdataoffset == 0)
	bdataoffset = etextpadoff;
    segpos[1] = segbase[1] = edataoffset = bdataoffset;
    combase[1] = segbase[1] + segsz[1];
#ifndef DATASEGS
    for (seg = 4; seg < NSEG; ++seg)
    {
	segpos[seg] = segbase[seg] = 0;
	combase[seg] = segbase[seg] + segsz[seg];
	segadj[seg] = etextpadoff;

	etextpadoff += ld_roundup(segsz[seg] + comsz[seg], 0x10, bin_off_t);
	segadj[1]   += ld_roundup(segsz[seg] + comsz[seg], 0x10, bin_off_t);
    }
    for (seg = 2; seg < 4; ++seg)
#else
    for (seg = 2; seg < NSEG; ++seg)
#endif
    {
	segpos[seg] = segbase[seg] = combase[seg - 1] + comsz[seg - 1];
        /* Do we want this for IY relative tricks on some builds on Z80 ? */
#if defined(MC6809) || defined(MC6502)
	if (seg == DPSEG)
	{
	    /* temporarily have fixed DP seg */
	    /* adjust if nec so it only spans 1 page */
	    tempoffset = segsz[seg] + comsz[seg];
	    if (tempoffset > 0x100)
		fatalerror("direct page segment too large");
	    if ((((segbase[seg] + tempoffset) ^ segbase[seg])
		 & ~(bin_off_t) 0xFF) != 0)
		segpos[seg] = segbase[seg] = (segbase[seg] + 0xFF)
					     & ~(bin_off_t) 0xFF;
	}
#endif

	combase[seg] = segbase[seg] + segsz[seg];
	segadj[seg] = segadj[seg - 1];
    }

    /* relocate symbols by offsets of segments in memory */
    for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    register struct symstruct **symparray;
	    register struct symstruct *symptr;

	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != NUL_PTR; ++symparray)
		if (symptr->modptr == modptr && !(symptr->flags & A_MASK))
		{
		    if (symptr->flags & (C_MASK | SA_MASK))
			    symptr->value += combase[symptr->flags & SEGM_MASK];
		    else
			symptr->value += segbase[symptr->flags & SEGM_MASK];
		}
	}

    /* adjust special symbols */
    for (seg = 0; seg < NSEG; ++seg)
    {
#ifdef DATASEGS
	if (segsz[seg] != 0)
	    /* only count data of nonzero length */
#else
	if (segsz[seg] != 0 && seg < 4)
#endif
	    edataoffset = segbase[seg] + segsz[seg];
	segboundary[5] = hexdigit[seg];		/* to __segX?H */
	segboundary[6] = 'D';
	setsym(segboundary, (tempoffset = segbase[seg]) + segsz[seg]);
						/* __segXDH */
	segboundary[7] = 'L';
	setsym(segboundary, tempoffset);	/* __segXDL */
	segboundary[6] = 'C';
	setsym(segboundary, tempoffset = combase[seg]);
						/* __segXCL */
	segboundary[7] = 'H';
	setsym(segboundary, tempoffset + comsz[seg]);
						/* __segXCH */
#ifndef DATASEGS
        if( seg > 3 )
	{
	   segboundary[6] = 'S';
	   segboundary[7] = 'O';
	   setsym(segboundary, (bin_off_t)(segadj[seg]-segadj[0])/0x10);
	   /* __segXSO */
        }
#endif
    }
    setsym("__etext", etextoffset);
    setsym("__edata", edataoffset);
#ifdef DATASEGS
    setsym("__end", endoffset = combase[NSEG - 1] + comsz[NSEG - 1]);
#else
    setsym("__end", endoffset = combase[3] + comsz[3]);
#endif
    setsym("__segoff", (bin_off_t)(segadj[1]-segadj[0])/0x10);
    if( !bits32 )
    {
        if( etextoffset > 65536L )
            fatalerror("text segment too large for 16bit");
        if( endoffset > 65536L )
            fatalerror("data segment too large for 16bit");
    }

    openout(outfilename);
    writeheader();
    for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext) {	if (modptr->loadflag)
	{
	    linkmod(modptr);
	    padmod(modptr);
	}
    }
    closeout();
    executable();
}

static void linkmod(struct modstruct *modptr)
{
    char buf[ABS_TEXT_MAX];
    int command;
    unsigned char modify;
    bin_off_t offset;
    int symbolnum;
    struct symstruct **symparray;
    struct symstruct *symptr;

    setseg(0);
    relocsize = 2;
    symparray = modptr->symparray;
    openin(modptr->filename);	/* does nothing if already open */
    seekin(modptr->textoffset);
    while (TRUE)
    {
	if ((command = readchar()) < 0)
	    prematureeof();
	modify = command & MODIFY_MASK;
	switch (command & CM_MASK)
	{
	case CM_SPECIAL:
	    switch (modify)
	    {
	    case CM_EOT:
		segpos[curseg] = spos;
		return;
	    case CM_BYTE_SIZE:
		relocsize = 1;
		break;
	    case CM_WORD_SIZE:
		relocsize = 2;
		break;
	    case CM_LONG_SIZE:
#ifdef LONG_OFFSETS
		relocsize = 4;
		break;
#else
		fatalerror("relocation by long offsets not implemented");
#endif
	    case CM_1_SKIP:
		skip(1);
		break;
	    case CM_2_SKIP:
		skip(2);
		break;
	    case CM_4_SKIP:
		skip(4);
		break;
	    default:
		if ((modify -= CM_0_SEG) >= NSEG)
		    inputerror("bad data in");
		setseg(modify);
		break;
	    }
	    break;
	case CM_ABSOLUTE:
	    if (modify == 0)
		modify = ABS_TEXT_MAX;
	    readin(buf, (unsigned) modify);
	    writeout(buf, (unsigned) modify);
	    spos += (int) modify;
	    break;
	case CM_OFFSET_RELOC:
	    offset = readsize(relocsize);
	    if (modify & R_MASK)
	    {
#ifndef DATASEGS
                int m = (modify & SEGM_MASK);
	        if( curseg != m && m != SEGM_MASK )
	           interseg(modptr->filename, modptr->archentry, (char*)0);
#endif
		offset -= (spos + relocsize);
            }
	    offtocn(buf, segbase[modify & SEGM_MASK] + offset, relocsize);
	    writeout(buf, relocsize);
	    spos += relocsize;
	    break;
	case CM_SYMBOL_RELOC:
	    symptr = symparray[symbolnum = readconvsize((unsigned)
					    (modify & S_MASK ? 2 : 1))];
	    offset = readconvsize((unsigned) modify & OF_MASK);
	    if (modify & R_MASK)
	    {
#ifndef DATASEGS
                int m = (symptr->flags & SEGM_MASK);
	        if( curseg != m && m != SEGM_MASK )
	           interseg(modptr->filename, modptr->archentry, symptr->name);
#endif
		offset -= (spos + relocsize);
	    }
		offset += symptr->value;	    
	    offtocn(buf, offset, relocsize);
	    writeout(buf, relocsize);
	    spos += relocsize;
	}
    }
}

static void padmod(struct modstruct *modptr)
{
    bin_off_t count;
    fastin_t seg;
    bin_off_t size;
    unsigned sizecount;
    char *sizeptr;

    for (seg = 0, sizeptr = modptr->segsize; seg < NSEG; ++seg)
    {
	size = cntooffset(sizeptr,
			  sizecount = segsizecount((unsigned) seg, modptr));
	sizeptr += sizecount;
	if ((count = segpos[seg] - segbase[seg]) != size)
	    size_error(seg, count, size);

	/* pad to quad boundary */
	/* not padding in-between common areas which sometimes get into file */
	if ((size = ld_roundup(segpos[seg], 4, bin_off_t) - segpos[seg]) != 0)
	{
	    setseg(seg);
	    writenulls(size);
	    segpos[seg] = spos;
	}
	segbase[seg] = segpos[seg];
    }
}

static void setsym(char *name, bin_off_t value)
{
    struct symstruct *symptr;

    if ((symptr = findsym(name)) != NUL_PTR)
        symptr->value = value;
}

static void symres(char *name)
{
    register struct symstruct *symptr;

    if ((symptr = findsym(name)) != NUL_PTR)
    {
	if ((symptr->flags & SEGM_MASK) == SEGM_MASK)
	    symptr->flags &= ~SEGM_MASK | curseg;
	if (symptr->flags != (I_MASK | curseg) || symptr->value != 0)
	    reserved(name);
        symptr->flags = E_MASK | curseg;	/* show defined, not common */
    }
}

/* set new segment */

static void setseg(fastin_pt newseg)
{
    if (newseg != curseg)
    {
	segpos[curseg] = spos;
	spos = segpos[curseg = newseg];
	seekout(FILEHEADERLENGTH + (unsigned long) spos
		+ (unsigned long) segadj[curseg]);
    }
}

static void skip(unsigned countsize)
{
    writenulls((bin_off_t) readsize(countsize));
}

static void writeheader(void)
{
}

static void writenulls(bin_off_t count)
{
    long lcount = count;
    if( lcount < 0 )
    	fatalerror("org command requires reverse seek");
    spos += count;
    while (count-- > 0)
	writechar(0);
}

