/* writex86.c - write binary file for linker */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "x86_aout.h"
#ifndef VERY_SMALL_MEMORY
#include "v7_aout.h"
#endif
#ifndef MSDOS
#include "x86_cpm86.h"
#endif
#include "const.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#define btextoffset (text_base_value)
#define bdataoffset (data_base_value)
#define page_size() ((bin_off_t)4096)

#ifndef ELF_SYMS
#define ELF_SYMS 0
#endif

#ifdef MSDOS
#  define FILEHEADERLENGTH (headerless?0:A_MINHDR)
#else
# ifdef VERY_SMALL_MEMORY
#  define FILEHEADERLENGTH (headerless?0:(cpm86?CPM86_HEADERLEN:A_MINHDR))
# else
#  define FILEHEADERLENGTH (headerless?0:(cpm86?CPM86_HEADERLEN:(v7?V7_HEADERLEN:A_MINHDR)))
# endif
#endif
				/* part of header not counted in offsets */
#define DPSEG 2

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

#define offsetof(struc, mem) ((int) &((struc *) 0)->mem)
#define memsizeof(struc, mem) sizeof(((struc *) 0)->mem)

PRIVATE bool_t bits32;		/* nonzero for 32-bit executable */
PRIVATE bin_off_t combase[NSEG];/* bases of common parts of segments */
PRIVATE bin_off_t comsz[NSEG];	/* sizes of common parts of segments */
PRIVATE fastin_t curseg;	/* current segment, 0 to $F */
PRIVATE bin_off_t edataoffset;	/* end of data */
PRIVATE bin_off_t endoffset;	/* end of bss */
PRIVATE bin_off_t etextoffset;	/* end of text */
PRIVATE bin_off_t etextpadoff;	/* end of padded text */
PRIVATE unsigned nsym;		/* number of symbols written */
PRIVATE unsigned relocsize;	/* current relocation size 1, 2 or 4 */
PRIVATE bin_off_t segadj[NSEG];	/* adjusts (file offset - seg offset) */
				/* depends on zero init */
PRIVATE bin_off_t segbase[NSEG];/* bases of data parts of segments */
PRIVATE char segboundary[9] = "__seg0DH";
				/* name of seg boundary __seg0DL to __segfCH */
PRIVATE bin_off_t segpos[NSEG];	/* segment positions for current module */
PRIVATE bin_off_t segsz[NSEG];	/* sizes of data parts of segments */
				/* depends on zero init */
PRIVATE bool_t sepid;		/* nonzero for separate I & D */
PRIVATE bool_t stripflag;	/* nonzero to strip symbols */
PRIVATE bin_off_t spos;		/* position in current seg */
PRIVATE bool_t uzp;		/* nonzero for unmapped zero page */
PRIVATE bool_t xsym;		/* extended symbol table */

FORWARD void linkmod P((struct modstruct *modptr));
FORWARD void padmod P((struct modstruct *modptr));
FORWARD void setsym P((char *name, bin_off_t value));
FORWARD void symres P((char *name));
FORWARD void setseg P((fastin_pt newseg));
FORWARD void skip P((unsigned countsize));
FORWARD void writeheader P((void));
#ifndef VERY_SMALL_MEMORY
FORWARD void v7header P((void));
#endif
#ifndef MSDOS
FORWARD void cpm86header P((void));
#endif
FORWARD void writenulls P((bin_off_t count));

EXTERN bool_t reloc_output;

/* write binary file */

PUBLIC void write_elks(outfilename, argsepid, argbits32, argstripflag, arguzp, argxsym)
char *outfilename;
bool_pt argsepid;
bool_pt argbits32;
bool_pt argstripflag;
bool_pt arguzp;
bool_pt argxsym;
{
    char buf4[4];
    char *cptr;
    struct nlist extsym;
    flags_t flags;
    struct modstruct *modptr;
    fastin_t seg;
    unsigned sizecount;
    bin_off_t tempoffset;

    if( reloc_output )
#ifndef MSDOS
       fatalerror("Output binformat not configured relocatable, use -N");
#else
       fatalerror("Cannot use -r under MSDOS, sorry");
#endif

    sepid = argsepid;
    bits32 = argbits32;
    stripflag = argstripflag;
    uzp = arguzp;
    xsym = argxsym;
    if (uzp)
    {
	if (btextoffset == 0)
	    btextoffset = page_size();
	if (bdataoffset == 0 && sepid)
	    bdataoffset = page_size();
    }

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
    curseg = 3;
    symres("__edata");
    symres("__end");
    symres("__heap_top");
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
		    {
			    undefined(symptr->name);
		    }
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

		/* adjust sizes to even to get quad boundaries */
		/* this should be specifiable dynamically */
		segsz[seg] = ld_roundup(segsz[seg], 4, bin_off_t);
		comsz[seg] = ld_roundup(comsz[seg], 4, bin_off_t);
		cptr += sizecount;
	    }
	}

    /* calculate seg positions now their sizes are known */
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
    if (sepid)
    {
	etextpadoff = ld_roundup(etextoffset, 0x10, bin_off_t);
	segadj[1] += etextpadoff - bdataoffset;
    }
    else if (bdataoffset == 0)
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
#ifdef MC6809
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

    if( heap_top_value < 0x100 || endoffset > heap_top_value-0x100)
       heap_top_value = endoffset + 0x8000;
    if( heap_top_value > 0x10000 && !bits32 ) heap_top_value = 0x10000;
    setsym("__heap_top", (bin_off_t)heap_top_value);

    openout(outfilename);
#ifndef MSDOS
    if (cpm86) cpm86header();
    else
#endif
#ifndef VERY_SMALL_MEMORY
    if (v7)
       v7header();
    else
#endif
       writeheader();
    for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    linkmod(modptr);
	    padmod(modptr);
	}

    /* dump symbol table */
    if (!stripflag)
    {
	seekout(FILEHEADERLENGTH
		+ (unsigned long) (etextpadoff - btextoffset)
		+ (unsigned long) (edataoffset - bdataoffset)
		);
	extsym.n_numaux = extsym.n_type = 0;
	for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext)
	    if (modptr->loadflag)
	    {
		register struct symstruct **symparray;
		register struct symstruct *symptr;

		for (symparray = modptr->symparray;
		     (symptr = *symparray) != NUL_PTR; ++symparray)
		    if (symptr->modptr == modptr)
		    {
#if ELF_SYMS
			if (symptr->name[0] == '_' && symptr->name[1] )
			  strncpy((char *) extsym.n_name, symptr->name+1,
				sizeof extsym.n_name);
			else
			{
			  memcpy((char *) extsym.n_name, "$", 1);
			  strncpy((char *) extsym.n_name+1, symptr->name,
				sizeof(extsym.n_name)-1);
			}
#else
			strncpy((char *) extsym.n_name, symptr->name,
				sizeof extsym.n_name);
#endif
			u4cn((char *) &extsym.n_value, (u4_t) symptr->value,
			     sizeof extsym.n_value);
			if ((flags = symptr->flags) & A_MASK)
			    extsym.n_sclass = N_ABS;
			else if (flags & (E_MASK | I_MASK))
			    extsym.n_sclass = C_EXT;
			else
			    extsym.n_sclass = C_STAT;
			if (!(flags & I_MASK) ||
			     flags & C_MASK)
			    switch (flags & (A_MASK | SEGM_MASK))
			    {
#ifdef DATASEGS
			    case 0:
#else
			    default:
#endif
				extsym.n_sclass |= N_TEXT;
			    case A_MASK:
				break;
#ifdef DATASEGS
			    default:
#else
			    case 1: case 2: case 3:
			    case A_MASK|1: case A_MASK|2: case A_MASK|3:
#endif
				if (flags & (C_MASK | SA_MASK))
				    extsym.n_sclass |= N_BSS;
				else
				    extsym.n_sclass |= N_DATA;
				break;
			    }
			writeout((char *) &extsym, sizeof extsym);
			++nsym;
#if !ELF_SYMS
			if( xsym )
			{
			   int i;
			   extsym.n_sclass = 0;
			   memset((void*)&extsym.n_value,0,
				   sizeof(extsym.n_value));

			   for(i=sizeof extsym.n_name; i<strlen(symptr->name);
			       i+=sizeof extsym.n_name)
			   {
			      strncpy((char *) extsym.n_name, symptr->name+i,
				sizeof extsym.n_name);
			      writeout((char *) &extsym, sizeof extsym);
			      ++nsym;
			   }
			}
#endif
		    }
	    }
	seekout((unsigned long) offsetof(struct exec, a_syms));
	u4cn(buf4, (u4_t) nsym * sizeof extsym,
	     memsizeof(struct exec, a_syms));
	writeout(buf4, memsizeof(struct exec, a_syms));
    }
    closeout();
    executable();
}

PRIVATE void linkmod(modptr)
struct modstruct *modptr;
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

PRIVATE void padmod(modptr)
struct modstruct *modptr;
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

PRIVATE void setsym(name, value)
char *name;
bin_off_t value;
{
    struct symstruct *symptr;

    if ((symptr = findsym(name)) != NUL_PTR)
	symptr->value = value;
}

PRIVATE void symres(name)
register char *name;
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

PRIVATE void setseg(newseg)
fastin_pt newseg;
{
    if (newseg != curseg)
    {
	segpos[curseg] = spos;
	spos = segpos[curseg = newseg];
	seekout(FILEHEADERLENGTH + (unsigned long) spos
		+ (unsigned long) segadj[curseg]);
    }
}

PRIVATE void skip(countsize)
unsigned countsize;
{
    writenulls((bin_off_t) readsize(countsize));
}

#ifndef MSDOS
PRIVATE void cpm86header()
{
    struct cpm86_exec header;
    memset(&header, 0, sizeof header);

    if (sepid)
    {
      header.ce_group[0].cg_type = CG_CODE;
      u2c2(header.ce_group[0].cg_len, (15 + etextpadoff) / 16);
      u2c2(header.ce_group[0].cg_min, (15 + etextpadoff) / 16);
      header.ce_group[1].cg_type = CG_DATA;
      u2c2(header.ce_group[1].cg_len, (15 + edataoffset) / 16);
      u2c2(header.ce_group[1].cg_min, (15 + endoffset  ) / 16);
      u2c2(header.ce_group[1].cg_max, 0x1000);
    }
    else
    {
      header.ce_group[0].cg_type = CG_CODE;
      u2c2(header.ce_group[0].cg_len, (15 + edataoffset) / 16);
      u2c2(header.ce_group[0].cg_min, (15 + endoffset  ) / 16);
    }
    if( FILEHEADERLENGTH )
       writeout((char *) &header, FILEHEADERLENGTH);
}
#endif

PRIVATE void writeheader()
{
    struct exec header;

    memset(&header, 0, sizeof header);
    header.a_magic[0] = A_MAGIC0;
    header.a_magic[1] = A_MAGIC1;
    header.a_flags = sepid ? A_SEP : A_EXEC;
    if (uzp)
	header.a_flags |= A_UZP;
    header.a_cpu = bits32 ? A_I80386 : A_I8086;
    header.a_hdrlen = FILEHEADERLENGTH;
    offtocn((char *) &header.a_text, etextpadoff - btextoffset,
	    sizeof header.a_text);
    offtocn((char *) &header.a_data, edataoffset - bdataoffset,
	    sizeof header.a_data);
    offtocn((char *) &header.a_bss, endoffset - edataoffset,
	    sizeof header.a_bss);
    if (uzp)
	offtocn((char *) &header.a_entry, page_size(),
		sizeof header.a_entry);

    offtocn((char *) &header.a_total, (bin_off_t) heap_top_value,
	    sizeof header.a_total);
    if( FILEHEADERLENGTH )
       writeout((char *) &header, FILEHEADERLENGTH);
}

#ifndef VERY_SMALL_MEMORY
PRIVATE void v7header()
{
    struct v7_exec header;

    if( sizeof header != FILEHEADERLENGTH )
       fatalerror("Executable miscompiled, computed wrong header size");

    memset(&header, 0, sizeof header);

    if( bits32 )
       fatalerror("V7 a.out format is for 16-bit only");

    offtocn((char *) &header.magic, sepid ? V7_MAGIC3 : V7_OMAGIC,
            sizeof header.magic);

    offtocn((char *) &header.textsize, etextpadoff - btextoffset,
            sizeof header.textsize);
    offtocn((char *) &header.datasize, edataoffset - bdataoffset,
            sizeof header.datasize);
    offtocn((char *) &header.bsssize, endoffset - edataoffset,
            sizeof header.bsssize);

    if( !stripflag )
       fatalerror("Symbol table not implemented for V7 yet");

    if( uzp )
       fatalerror("No QMAGIC for V7");

    offtocn((char *) &header.entry, entryfirst->elsymptr->value,
            sizeof header.entry);

    if( FILEHEADERLENGTH )
       writeout((char *) &header, FILEHEADERLENGTH);
}
#endif

PRIVATE void writenulls(count)
bin_off_t count;
{
    long lcount = count;
    spos += count;
#if 0
    /* This will only work if we record the highest spos found an seek there
     * at the end of the generation
     */

    seekout(FILEHEADERLENGTH + (unsigned long) spos
	 + (unsigned long) segadj[curseg]);
    return;

#endif
    if( lcount < 0 )
    	fatalerror("org command requires reverse seek");
    while (count-- > 0)
	writechar(0);
}
