/* readobj.c - read object file for linker */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "ar.h"			/* maybe local copy of <ar.h> for cross-link */
#include "const.h"
#include "byteord.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

/*
   Linking takes 2 passes. The 1st pass reads through all files specified
in the command line, and all libraries. All public symbols are extracted
and stored in a chained hash table. For each module, its file and header
data recorded, and the resulting structures are chained together
(interleaved with the symbols).

   The symbol descriptors are separated from the symbol names, so we must
record all the descriptors of a module before putting the symbols in the
symbol table (poor design). The descriptors are stored in the symbol
table, then moved to the top of the table to make room for the symols.
The symbols referred to in a given module are linked together by a chain
beginning in the module descriptor.
*/

PRIVATE unsigned convertsize[NSEG / 4] = {0, 1, 2, 4};
PRIVATE struct entrylist *entrylast;	/* last on list of entry symbols */
PRIVATE struct redlist *redlast;	/* last on list of redefined symbols */
PRIVATE struct modstruct *modlast;	/* data for last module */

FORWARD long readarheader P((char **parchentry));
FORWARD unsigned readfileheader P((void));
FORWARD void readmodule P((char *filename, char *archentry));
FORWARD void reedmodheader P((void));
FORWARD bool_pt redsym P((struct symstruct *symptr, bin_off_t value));
FORWARD unsigned checksum P((char *string, unsigned length));
FORWARD unsigned segbits P((unsigned seg, char *sizedesc));

/* initialise object file handler */

PUBLIC void objinit()
{
    modfirst = modlast = NUL_PTR;
    entryfirst = entrylast = NUL_PTR;
    redfirst = redlast = NUL_PTR;
}

/* read all symbol definitions in an object file */

PUBLIC void readsyms(filename, trace)
char *filename;
bool_pt trace;
{
    char *archentry;
    long filelength;
    char filemagic[SARMAG];
    unsigned long filepos;
    unsigned modcount;

    if (trace)
	errtrace(filename, 0);
    openin(filename);		/* input is not open, so position is start */
    switch ((unsigned) readsize(2))
    {
    case OMAGIC:
	seekin((unsigned long) 0);
	for (modcount = readfileheader(); modcount-- != 0;)
	    readmodule(filename, (char *) NUL_PTR);
	break;
    default:
	seekin((unsigned long) 0);
	readin(filemagic, sizeof filemagic);
	if (strncmp(filemagic, ARMAG, sizeof filemagic) != 0)
	    input1error(" has bad magic number");
	filepos = SARMAG;
	while ((filelength = readarheader(&archentry)) > 0)
	{
	    unsigned int magic;
	    if (trace)
		errtrace(archentry, 2);
	    filepos += sizeof(struct ar_hdr);
            magic = (unsigned) readsize(2);
            if(magic == OMAGIC)
	    {
	        seekin(filepos);
	        for (modcount = readfileheader(); modcount-- != 0;)
	        {
		    readmodule(stralloc(filename), archentry);
		    modlast->textoffset += filepos;
	        }
	    }
	    else if( magic == 0x3C21 ) /* "!<" */
	       filelength = SARMAG;
	    seekin(filepos += ld_roundup(filelength, 2, long));
	}
	break;
    }
    closein();
}

/* read archive header and return length */

PRIVATE long readarheader(parchentry)
char **parchentry;
{
    struct ar_hdr arheader;
    char *endptr;
    char *nameptr;

    if (readineofok((char *) &arheader, sizeof arheader))
	return 0;
    strncpy (*parchentry = nameptr = ourmalloc(sizeof arheader.ar_name + 1),
	     arheader.ar_name, sizeof arheader.ar_name);
    endptr = nameptr + sizeof arheader.ar_name;
    do
	*endptr-- = 0;
    while (endptr > nameptr && (*endptr == ' ' || *endptr == '/'));
    return strtoul(arheader.ar_size, (char **) NUL_PTR, 0);
}

/* read and check file header of the object file just opened */

PRIVATE unsigned readfileheader()
{
    struct
    {
	char magic[2];
	char count[2];		/* really an int */
    }
     fileheader;
    char filechecksum;		/* part of fileheader but would unalign */

    readin((char *) &fileheader, sizeof fileheader);
    readin(&filechecksum, sizeof filechecksum);
    if (filechecksum != checksum((char *) &fileheader, sizeof fileheader))
	input1error(" is not an object file (checksum failed)");
    return c2u2(fileheader.count);
}

/* read the next module */

PRIVATE void readmodule(filename, archentry)
char *filename;
char *archentry;
{
    struct symdstruct		/* to save parts of symbol before name known */
    {
	bin_off_t dvalue;
	flags_t dflags;
    };
    struct symdstruct *endsymdptr;
    flags_t flags;
    unsigned nsymbol;
    struct symdstruct *symdptr;
    char *symname;
    struct symstruct **symparray;
    struct symstruct *symptr;

    reedmodheader();
    modlast->filename = filename;
    modlast->archentry = archentry;
    nsymbol = readsize(2);
    symdptr = (struct symdstruct *)
	ourmalloc(nsymbol * sizeof(struct symdstruct));
    for (endsymdptr = symdptr + nsymbol; symdptr < endsymdptr; ++symdptr)
    {
	readsize(2);		/* discard string offset, assume strings seq */
	symdptr->dflags = flags = readsize(2);
	symdptr->dvalue = readconvsize((flags & SZ_MASK) >> SZ_SHIFT);
	/* NB unsigned flags to give logical shift */
	/* bug in Xenix 2.5 cc causes (int) of the */
	/* argument to turn flags into an int */
    }
    symdptr = (struct symdstruct *)
	moveup(nsymbol * sizeof(struct symdstruct));
    modlast->symparray = symparray = (struct symstruct **)
	ourmalloc((nsymbol + 1) * sizeof(struct symstruct *));
    symname = readstring();	/* module name */
    modlast->modname = stralloc(symname);	/* likely OK overlapped copy */
    for (endsymdptr = symdptr + nsymbol; symdptr < endsymdptr;
	 *symparray++ = symptr, release((char *) ++symdptr))
    {
	symname = readstring();
	if ((flags = symdptr->dflags) & (E_MASK | I_MASK) &&
	    (symptr = findsym(symname)) != NUL_PTR)
	{
	    /*
	       weaken segment-checking by letting the maximum segment
	       (SEGM_MASK) mean any segment
	    */
	    if ((symptr->flags & SEGM_MASK) == SEGM_MASK)
		symptr->flags &= ~(flags_t) SEGM_MASK | (flags & SEGM_MASK);
	    else if ((flags & SEGM_MASK) == SEGM_MASK)
		flags &= ~(flags_t) SEGM_MASK | (symptr->flags & SEGM_MASK);
	    if ((flags ^ symptr->flags) & (A_MASK | SEGM_MASK))
	    {
		redefined(symname, " with different segment or relocatability",
			  archentry, symptr->modptr->filename,
			  symptr->modptr->archentry);
		continue;
	    }
	    if (symptr->flags & E_MASK)
	    {
		if (flags & E_MASK && redsym(symptr, symdptr->dvalue))
		    redefined(symname, "", archentry, symptr->modptr->filename,
			      symptr->modptr->archentry);
		continue;
	    }
	    if (flags & I_MASK && symdptr->dvalue <= symptr->value)
		continue;
	}
	else
	    symptr = addsym(symname);
	symptr->modptr = modlast;
	symptr->value = symdptr->dvalue;
	symptr->flags = flags;
	if (flags & N_MASK)
	    entrysym(symptr);
    }
    *symparray = NUL_PTR;
}

/* put symbol on entry symbol list if it is not already */

PUBLIC void entrysym(symptr)
struct symstruct *symptr;
{
    register struct entrylist *elptr;

    for (elptr = entryfirst; elptr != NUL_PTR; elptr = elptr->elnext)
	if (symptr == elptr->elsymptr)
	    return;
    elptr = (struct entrylist *) ourmalloc(sizeof(struct entrylist));
    elptr->elnext = NUL_PTR;
    elptr->elsymptr = symptr;
    if (entryfirst == NUL_PTR)
	entryfirst = elptr;
    else
	entrylast->elnext = elptr;
    entrylast = elptr;
}

/* read the header of the next module */

PRIVATE void reedmodheader()
{
    struct
    {
	char htextoffset[4];	/* offset to module text in file */
	char htextsize[4];	/* size of text (may be 0 for last mod) */
	char stringssize[2];	/* size of string area */
	char hclass;		/* module class */
	char revision;		/* module revision */
    }
     modheader;
    unsigned seg;
    unsigned count;
    char *cptr;
    struct modstruct *modptr;

    readin((char *) &modheader, sizeof modheader);
    modptr = (struct modstruct *) ourmalloc(sizeof(struct modstruct));
    modptr->modnext = NUL_PTR;
    modptr->textoffset = c4u4(modheader.htextoffset);
    modptr->class = modheader.hclass;
    readin(modptr->segmaxsize, sizeof modptr->segmaxsize);
    readin(modptr->segsizedesc, sizeof modptr->segsizedesc);
    cptr = modptr->segsize;
    for (seg = 0; seg < NSEG; ++seg)
    {
	if ((count = segsizecount(seg, modptr)) != 0)
	{
	    if (cptr == modptr->segsize)
		ourmalloc(count - 1);	/* 1st byte reserved in struct */
	    else
		ourmalloc(count);
	    readin(cptr, count);
	    cptr += count;
	}
    }
    if (modfirst == NUL_PTR)
	modfirst = modptr;
    else
	modlast->modnext = modptr;
    modlast = modptr;
}

PRIVATE bool_pt redsym(symptr, value)
register struct symstruct *symptr;
bin_off_t value;
{
    register struct redlist *rlptr;
    char class;

    if (symptr->modptr->class != (class = modlast->class))
	for (rlptr = redfirst;; rlptr = rlptr->rlnext)
	{
	    if (rlptr == NUL_PTR)
	    {
		rlptr = (struct redlist *)
		    ourmalloc(sizeof(struct redlist));
		rlptr->rlnext = NUL_PTR;
		rlptr->rlsymptr = symptr;
		if (symptr->modptr->class < class)
		    /* prefer lower class - put other on redlist */
		{
		    rlptr->rlmodptr = modlast;
		    rlptr->rlvalue = value;
		}
		else
		{
		    rlptr->rlmodptr = symptr->modptr;
		    symptr->modptr = modlast;
		    rlptr->rlvalue = symptr->value;
		    symptr->value = value;
		}
		if (redfirst == NUL_PTR)
		    redfirst = rlptr;
		else
		    redlast->rlnext = rlptr;
		redlast = rlptr;
		return FALSE;
	    }
	    if (symptr == rlptr->rlsymptr && class == rlptr->rlmodptr->class)
		break;
	}
    return TRUE;
}

PRIVATE unsigned checksum(string, length)
char *string;
unsigned length;
{
    unsigned char sum;		/* this is a 1-byte checksum */

    for (sum = 0; length-- != 0;)
	sum += *string++ & 0xFF;
    return sum;
}

PUBLIC bin_off_t readconvsize(countindex)
unsigned countindex;
{
    return readsize(convertsize[countindex]);
}

PUBLIC bin_off_t readsize(count)
unsigned count;
{
    char buf[MAX_OFFSET_SIZE];

    if (count == 0)
	return 0;
    readin(buf, count);
    return cntooffset(buf, count);
}

PRIVATE unsigned segbits(seg, sizedesc)
unsigned seg;
char *sizedesc;
{
    return 3 & ((unsigned) sizedesc[((NSEG - 1) - seg) / 4] >> (2 * (seg % 4)));
    /* unsigned to give logical shift */
}

PUBLIC unsigned segsizecount(seg, modptr)
unsigned seg;
struct modstruct *modptr;
{
    return convertsize[segbits(seg, modptr->segsizedesc)];
}
