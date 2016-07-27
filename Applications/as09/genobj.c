/* genobj.c - object code generation routines for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "address.h"
#include "file.h"
#include "globvar.h"

#define OBJBUFSIZE 512		/* size of object code output buffer */
#define isge2byteoffset(offset) ((offset) >= 0x100)
#define isge4byteoffset(offset) ((offset) >= 0x10000L)

static char hid_absbuf[OBJ_MAX_ABS_LEN];	/* absolute object code buf */
static char *absbuf;		/* start */
static char *absbufend;	/* last location + 1 */
static char *absbufptr;	/* current location */
static struct sym_s **arrext;	/* array of external symbol ptrs */
static char hid_objbuf[OBJBUFSIZE];	/* object code buffer */
static unsigned numext;	/* number of external symbols */
static char *objbuf;		/* start */
static char *objbufend;	/* last location + 1 */
static char *objbufptr;	/* current location */
static unsigned char relsize;	/* current relocation size, 0 init */
				/* local to genobjadr, but here */
				/* because of static re-init bug */
static offset_t rmbcount;	/* accumulator for repeated RMB's */

static void flushabs(void);
static void flushrmb(void);
static void genobjadr(struct address_s *adrptr, smallcount_t size);
static void putobj1(opcode_pt ch);
static void putobj4(u32_T offset);
static void putobjoffset(offset_t offset, count_t size);
static void putobjword(unsigned word);
static void writeobj(char *buf, unsigned count);

/* accumulate RMB requests into 1 (so + and - requests cancel) */

void accumulate_rmb(offset_t offset)
{
    if (objectc)
    {
	flushabs();
	rmbcount += offset;
    }
}

/* flush absolute object code buffer to object code buffer if necessary */

static void flushabs(void)
{
    if (absbufptr > absbuf)
    {
	putobj1((absbufptr - absbuf) | OBJ_ABS);
	{
	    register char *bufptr;

	    bufptr = absbuf;
	    do
		putobj1(*bufptr);
	    while (++bufptr < absbufptr);
	    absbufptr = absbuf;
	}
    }
}

/* flush object code buffer if necessary */

void flushobj(void)
{
    int ntowrite;

    if ((ntowrite = objbufptr - objbuf) > 0)
    {
	if (write(objfil, objbuf, (unsigned) ntowrite) != ntowrite)
	{
	    error(OBJOUT);
	    listline();
	    finishup();
	}
	objbufptr = objbuf;
    }
}

/* flush RMB count if necessary */

static void flushrmb(void)
{
    count_t size;

    if (rmbcount != 0)
    {
#if SIZEOF_OFFSET_T > 2
	if (isge4byteoffset(rmbcount))
	{
	    putobj1(OBJ_SKIP_4);
	    size = 4;
	}
	else
#endif
	if (isge2byteoffset(rmbcount))
	{
	    putobj1(OBJ_SKIP_2);
	    size = 2;
	}
	else
	{
	    putobj1(OBJ_SKIP_1);
	    size = 1;
	}
	putobjoffset(rmbcount, size);
	rmbcount = 0;
    }
}

/* generate object code for current line */

/*
  any address parameter is (regrettably) in lastexp
  any immediate parameter is (corectly) in immadr
*/

void genobj(void)
{
    struct address_s *adrptr;
    char *bufptr;
    unsigned char remaining;

    if (objectc && mcount != 0)
    {
	if (popflags)
	{
	    if (fcflag)
	    {
		bufptr = databuf.fcbuf;
		remaining = mcount;
		do
		    putabs(*bufptr++);
		while (--remaining != 0);
	    }
	    if (fdflag)
	    {
		adrptr = databuf.fdbuf;
		remaining = mcount;
		do
		    genobjadr(adrptr++, 2);
		while ((remaining -= 2) != 0);
	    }
#if SIZEOF_OFFSET_T > 2
	    if (fqflag)
	    {
		adrptr = databuf.fqbuf;
		remaining = mcount;
		do
		    genobjadr(adrptr++, 4);
		while ((remaining -= 4) != 0);
	    }
#endif
	}
	else
	{
	    remaining = mcount - 1;	/* count opcode immediately */
#ifdef I80386
	    if (aprefix != 0)
	    {
		putabs(aprefix);
		--remaining;
	    }
	    if (oprefix != 0)
	    {
		putabs(oprefix);
		--remaining;
	    }
	    if (sprefix != 0)
	    {
		putabs(sprefix);
		--remaining;
	    }
#endif
	    if (page != 0)
	    {
		putabs(page);
		--remaining;
	    }
	    putabs(opcode);
	    if (remaining != 0)
	    {
		if (postb != 0)
		{
		    putabs(postb);
		    --remaining;
		}
#ifdef I80386
		if (sib != NO_SIB)
		{
		    putabs(sib);
		    --remaining;
		}
#endif
		if (remaining != 0)
		    genobjadr(&lastexp, remaining);
	    }
	}
#ifdef I80386
	if (immcount != 0)
	    genobjadr(&immadr, immcount);
#endif
    }
}

/* generate object code for current address */

static void genobjadr(struct address_s *adrptr, smallcount_t size)
{
    unsigned char byte;
    unsigned symnum;

    if (!(adrptr->data & RELBIT))
    {
	/* absolute address */

	char buf[sizeof(offset_t)];

#if SIZEOF_OFFSET_T > 2
	u4cn(buf, adrptr->offset, size);
#else
	u2cn(buf, adrptr->offset, size);
#endif
	putabs(buf[0]);
	if (size > 1)
	    putabs(buf[1]);
	if (size > 2)
	{
	    putabs(buf[2]);
	    putabs(buf[3]);
	}
    }
    else
    {
	/* relocatable address */
	if (size != relsize)
	    /* set reloc size index |00|0000xx| */
	    putobj((relsize = size) == 4 ? 0x03 : relsize);
	if (!(adrptr->data & IMPBIT))
	{
	    /* offset relocation (known offset) */
	    putobj((adrptr->data & SEGM) | OBJ_OFFSET_REL | pcrflag);
	    putobjoffset(adrptr->offset, size);
	}
	else
	{
	    /* symbol relocation (imported symbol + offset) */
	    {
		register struct sym_s **copyptr;

		for (copyptr = arrext, symnum = 0;
		     symnum < numext && *copyptr++ != adrptr->sym; ++symnum)
		    ;
	    }
	    byte = OBJ_SYMBOL_REL;
	    if (isge2byteoffset(symnum))
		byte = OBJ_SYMBOL_REL | OBJ_S_MASK;
#if SIZEOF_OFFSET_T > 2
	    if (isge4byteoffset(adrptr->offset))
	    {
		byte |= 0x03;	/* 4 byte offset */
		size = 4;
	    }
	    else
#endif
	    if (isge2byteoffset(adrptr->offset))
	    {
		byte |= 0x02;	/* 2 byte offset */
		size = 2;
	    }
	    else if (adrptr->offset != 0)
	    {
		byte |= 0x01;	/* 1 byte offset */
		size = 1;
	    }
	    else
		size = 0;
	    putobj(byte | pcrflag);
	    if (isge2byteoffset(symnum))
		putobjword(symnum);
	    else
		putobj1((opcode_pt) symnum);
	    if (adrptr->offset != 0)
		putobjoffset(adrptr->offset, size);
	}
    }
}

/* initialise private variables */

void initobj(void)
{
    absbufend = (absbufptr = absbuf = hid_absbuf) + sizeof hid_absbuf;
    objbufend = (objbufptr = objbuf = hid_objbuf) + sizeof hid_objbuf;
}

/*
  write header to object file
  also build array of imported/exported symbols
*/

void objheader(void)
{
    static unsigned char module_header[] =
    {
#ifdef I80386
	0xA3, 0x86,
	1, 0,
	(char) (0xA3 + 0x86 + 1 + 0),
#endif
#ifdef MC6809
	'S', '1',		/* 2 byte magic number */
	0, 1,			/* 2 byte number of modules in file */
	'S' + '1' + 0 + 1,	/* 1 byte checksum */
#endif
    };
    static char seg_max_sizes[] =
    {
	0x55,			/* all segments have maximum size 2^16 */
	0x55,			/* this is encoded by 0b01 4 times per byte */
	0x55,			/* other codes are 0b00 = max size 2^8 */
	0x55,			/* 0b10 = max size 2^24, 0b11 = max 2^32 */
    };
    unsigned char byte;
    register struct sym_s **copyptr;
    struct sym_s **copytop;
    struct sym_s **hashptr;
    struct lc_s *lcp;
    char module_name[FILNAMLEN + 1];
    char *nameptr;
    unsigned offset;
    unsigned segsizebytes;
    unsigned size;
    unsigned char sizebits;
    unsigned strsiz;		/* size of object string table */
    unsigned symosiz;		/* size of object symbol table */
    register struct sym_s *symptr;
    u32_T textlength;
    int symcount = 0;

    if ((objectc = objectg) == 0)
	return;
    writeobj((char *)module_header, sizeof module_header);

    /* calculate number of imported/exported symbols */
    /* and lengths of symbol and string tables */
    /* build array of imported/exported symbols */

    symosiz = 0;
    if (truefilename == NUL_PTR)
	truefilename = filnamptr;
    nameptr = strrchr(truefilename, DIRCHAR);
    strcpy(module_name, nameptr != NUL_PTR ? nameptr + 1 : truefilename);
    if ((nameptr = strrchr(module_name, '.')) != NUL_PTR)
	*nameptr = 0;
    strsiz = strlen(module_name) + 1;
    
    for (hashptr = spt; hashptr < spt_top;)
	if ((symptr = *hashptr++) != NUL_PTR)
	    do
	    {
		if ((symptr->type & EXPBIT || symptr->data & IMPBIT) ||
		    (!globals_only_in_obj && symptr->name[0] != '.' &&
		    !(symptr->type & (MNREGBIT | MACBIT | VARBIT))))
		{
		    symcount ++;
		}
	    }
	    while ((symptr = symptr->next) != NUL_PTR);
    arrext = copyptr = asalloc( sizeof(struct sym_s *) * symcount);

    for (hashptr = spt; hashptr < spt_top;)
	if ((symptr = *hashptr++) != NUL_PTR)
	    do
	    {
		if ((symptr->type & EXPBIT || symptr->data & IMPBIT) ||
		    (!globals_only_in_obj && symptr->name[0] != '.' &&
		    !(symptr->type & (MNREGBIT | MACBIT | VARBIT))))
		{
		    *copyptr++ = symptr;
		    strsiz += symptr->length + 1;
		    if (textseg>=0 && (symptr->data & SEGM) == textseg)
		       strsiz+=2;
#if SIZEOF_OFFSET_T > 2
		    if (isge4byteoffset(symptr->value_reg_or_op.value))
			size = 4 + 4;
			/* 4 is size of offset into string table and flags */
			/* 2nd 4 is for 4 byte offset */
		    else
#endif
		    if (isge2byteoffset(symptr->value_reg_or_op.value))
			size = 4 + 2;
		    else if (symptr->value_reg_or_op.value != 0)
			size = 4 + 1;
		    else
			size = 4;
		    symosiz += size;
		    ++numext;
		}
	    }
	    while ((symptr = symptr->next) != NUL_PTR);
    copytop = copyptr;

    /* calculate length of text, and number of seg size bytes in header */

    textlength = segsizebytes = 0;
    lcp = lctab;
    do
	if (lcp->lc != 0)
	{
	    textlength += lcp->lc;	/* assuming text starts at 0 */
#if SIZEOF_OFFSET_T > 2
	    if (isge4byteoffset(lcp->lc))
		segsizebytes += 4;
	    else
#endif
		segsizebytes += 2;	/* use 2 byte size if possible */
	}
    while (++lcp < lctabtop);

/*
  offset to text = length of header since only 1 module
  header consists of:
  module header			sizeof module_header
  offset to start of text	4
  length of text		4
  length of string area		2
  class				1
  revision			1
  seg max sizes			sizeof seg_max_sizes
  seg size descriptors		4
  seg sizes			segsizebytes
  symbol count			2
  symbol offsets and types	symosiz
  strings			strsiz
*/

    /* offset to start of text */

    /* FIXME : on 16bit need to force maths to 32bit as well ? */
    putobj4((u32_T) (sizeof module_header + 4 + 4 + 2 + 1 + 1 +
		     sizeof seg_max_sizes + 4 + segsizebytes + 2 +
		     symosiz) + strsiz);

    /* length of text */

    putobj4((u32_T) textlength);

    /* length of string area */

    putobjword(strsiz);

    /* class and revision */

    putobj1(0);
    putobj1(0);

    /* segment max sizes (constant) */

    writeobj(seg_max_sizes, sizeof seg_max_sizes);

    /* segment size descriptors */
    /* produce only 0 and 2 byte sizes */

    lcp = lctabtop;
    byte = 0;
    sizebits = OBJ_SEGSZ_TWO << 6;
    do
    {
	--lcp;
	if (lcp->lc != 0)
	{
	    byte |= sizebits;
#if SIZEOF_OFFSET_T > 2
	    if (isge4byteoffset(lcp->lc))
		byte |= sizebits >> 1;	/* XXX - convert size 2 to size 4 */
#endif
	}
	if ((sizebits >>= 2) == 0)
	{
	    putobj1(byte);
	    byte = 0;
	    sizebits = OBJ_SEGSZ_TWO << 6;
	}
    }
    while (lcp > lctab);

    /* segment sizes */

    do				/* lcp starts at lctab */
	if (lcp->lc != 0)
	{
#if SIZEOF_OFFSET_T > 2
	    if (isge4byteoffset(lcp->lc))
		putobj4(lcp->lc);
	    else
#endif
		putobjword((unsigned) lcp->lc);
	}
    while (++lcp < lctabtop);

    /* symbol count */

    putobjword(numext);

    /* symbol offsets and types */

    offset = strlen(module_name) + 1;	/* 1st symbol begins after name */
    for (copyptr = arrext; copyptr < copytop;)
    {
	putobjword(offset);
	symptr = *copyptr++;
	byte = symptr->type & OBJ_N_MASK;
#if SIZEOF_OFFSET_T > 2
	if (isge4byteoffset(symptr->value_reg_or_op.value))
	{
	    byte |= OBJ_SZ_FOUR;
	    size = 4;
	}
	else
#endif
	if (isge2byteoffset(symptr->value_reg_or_op.value))
	{
	    byte |= OBJ_SZ_TWO;
	    size = 2;
	}
	else if (symptr->value_reg_or_op.value != 0)
	{
	    byte |= OBJ_SZ_ONE;
	    size = 1;
	}
	else
	    size = 0;
	if ((symptr->type & (COMMBIT | REDBIT)) == (COMMBIT | REDBIT))
	{
	    byte |= OBJ_SA_MASK;
	    symptr->data &= ~OBJ_I_MASK;
	}
	putobjword((unsigned)
		   (byte << 0x8) |
		   (symptr->type & OBJ_E_MASK) |	/* |E|0000000| */
	       ((symptr->data & (OBJ_I_MASK | OBJ_A_MASK | OBJ_SEGM_MASK)) ^
	/* |0|I|0|A|SEGM| */
		RELBIT));	/* RELBIT by negative logic */
	if ((symptr->type & (COMMBIT | REDBIT)) == (COMMBIT | REDBIT))
	    symptr->data |= OBJ_I_MASK;
	if (size != 0)
	    putobjoffset(symptr->value_reg_or_op.value, size);
	offset += symptr->length + 1;
	if (textseg>=0 && (symptr->data & SEGM) == textseg)
	   offset+=2;
    }

    /* strings */

    writeobj(module_name, strlen(module_name));
    putobj1(0);
    for (copyptr = arrext; copyptr < copytop;)
    {
	symptr = *copyptr++;
	writeobj(symptr->name, symptr->length);
	if (textseg>=0 && (symptr->data & SEGM) == textseg)
	{
	   putobj1('.');
	   putobj1(hexdigit[textseg]);
	}
	putobj1(0);
    }
    putobj1(OBJ_SET_SEG | 0);	/* default segment 0, |0010|SEGM| */
}

/* write trailer to object file */

void objtrailer(void)
{
    if (objectc)
    {
	putobj(0);		/* end of object file */
	flushobj();
    }
}

/* write char to absolute object code buffer, flush if necessary */

void putabs(opcode_pt ch)
{
    if (objectc)
    {
	if (rmbcount != 0)
	    flushrmb();
	if (absbufptr >= absbufend)
	    flushabs();
	*absbufptr++ = ch;
    }
}

/* write char to object code buffer, flush if necessary */

void putobj(opcode_pt ch)
{
    if (objectc)
    {
	flushabs();
	flushrmb();
	putobj1(ch);
    }
}

/* write char to object code buffer assuming nothing in absolute & rmb bufs */

static void putobj1(opcode_pt ch)
{
    if (objbufptr >= objbufend)
	flushobj();
    *objbufptr++ = ch;
}

/* write 32 bit offset to object code buffer assuming ... */

static void putobj4(u32_T offset)
{
    char buf[sizeof offset];

    u4c4(buf, offset);
    writeobj(buf, 4);
}

/* write sized offset to object code buffer assuming ... */

static void putobjoffset(offset_t offset, count_t size)
{
    char buf[sizeof offset];

#if SIZEOF_OFFSET_T > 2
    u4cn(buf, offset, size);
#else
    u2cn(buf, offset, size);
#endif
    putobj1(buf[0]);
    if (size > 1)
	putobj1(buf[1]);
    if (size > 2)
    {
	putobj1(buf[2]);
	putobj1(buf[3]);
    }
}

/* write word to object code buffer assuming ... */

static void putobjword(unsigned word)
{
    char buf[sizeof word];

    u2c2(buf, word);
    putobj1(buf[0]);
    putobj1(buf[1]);
}

/* write several bytes to object code buffer assuming ... */

static void writeobj(char *buf, unsigned count)
{
    do
	putobj1(*buf++);
    while (--count);
}
