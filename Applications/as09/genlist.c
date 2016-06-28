/* genlist.c - generate listing and error reports for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "address.h"
#include "flag.h"
#include "file.h"
#include "globvar.h"
#include "macro.h"
#include "scan.h"
#include "source.h"

#define CODE_LIST_LENGTH (sizeof (struct code_listing_s) - 1)
				/* length of formatted code listing */
#define MAXERR 6		/* maximum errors listed per line */

struct error_s			/* to record error info */
{
    char * err_str;
    unsigned char position;
};

/* code listing format */

struct code_listing_s
{
    union linum_macro_u
    {
	char linum[LINUM_LEN];
	struct
	{
	    char pad[1];
	    char mark[1];
	    char level[1];
	}
	 macro;
    }
     linum_or_macro;
    char padlinum[1];
    char lc[4];
    char padlc[1];
#ifdef I80386
    char lprefix[2];
    char aprefix[2];
    char oprefix[2];
    char sprefix[2];
#endif
    char page[2];
    char opcode[2];
    char postb[2];
#ifdef I80386
    char sib[2];
#endif
    char padopcode[1];
#if SIZEOF_OFFSET_T > 2
    char displ4[2];
    char displ3[2];
#endif
    char displ2[2];
    char displ1[2];
    char reldispl[1];
    char paddispl[1];
#ifdef I80386
    char imm4[2];
    char imm3[2];
    char imm2[2];
    char imm1[2];
    char relimm[1];
    char padimm[1];
#endif
    char nullterm;
};

static struct error_s errbuf[MAXERR];	/* error buffer */
static unsigned char errcount;	/* # errors in line */
static bool_t erroverflow;	/* set if too many errors on line */

static char *build_1hex_number(opcode_pt num, char *where);
static void list1(fd_t fd);
static void listcode(void);
static void listerrors(void);
static void paderrorline(unsigned nspaces);

/* format 1 byte number as 2 hex digits, return ptr to end */

static char *build_1hex_number(opcode_pt num, register char *where)
{
    where[0] = hexdigit[((unsigned) num % 256) / 16];
    where[1] = hexdigit[(unsigned) num % 16];
    return where + 2;
}

/* format 2 byte number as 4 hex digits, return ptr to end */

char *build_2hex_number(unsigned num, char *where)
{
    return build_1hex_number((opcode_pt) num,
			     build_1hex_number((opcode_pt) (num / 256), where));
}

/* format 2 byte number as decimal with given width (pad with leading '0's) */
/* return ptr to end */

char *build_number(unsigned num, unsigned width, register char *where)
{
    static unsigned powers_of_10[] = {1, 10, 100, 1000, 10000,};
    unsigned char digit;
    unsigned char power;
    register unsigned power_of_10;

    power = 5;			/* actually 1 more than power */
    do
    {
	for (digit = '0', power_of_10 = (powers_of_10 - 1)[power];
	     num >= power_of_10; num -= power_of_10)
	    ++digit;
	if (power <= width)
	    *where++ = digit;
    }
    while (--power != 0);
    return where;
}

/* record number and position of error (or error buffer overflow) */

void warning(char * err_str)
{
    if (!as_warn.current) return;
    ++totwarn;
    --toterr;
    error(err_str);
}

void error(char *err_str)
{
    register struct error_s *errptr;
    register struct error_s *errptrlow;
    unsigned char position;

    if (errcount >= MAXERR)
	erroverflow = TRUE;
    else
    {
	position = symname - linebuf;
	for (errptr = errbuf + errcount;
	     errptr > errbuf && errptr->position > position;
	     errptr = errptrlow)
	{
	    errptrlow = errptr - 1;
	    errptr->err_str = errptrlow->err_str;
	    errptr->position = errptrlow->position;
	}
	errptr->err_str = err_str;
	errptr->position = position;
	++errcount;
	++toterr;
    }
}

/* list 1 line to list file if any errors or flags permit */
/* list line to console as well if any errors and list file is not console */

void listline(void)
{
    if (!listpre && lineptr != 0)
    {
	if (errcount || (list.current && (!macflag || mcount != 0)) ||
	    (macflag && maclist.current) || list_force )
	    list1(lstfil);
	if (errcount)
	{
	    if (lstfil != STDOUT)
		list1(STDOUT);
	    errcount = 0;
	    erroverflow = FALSE;
	}
    }
}

/* list 1 line unconditionally */

static void list1(fd_t fd)
{
    outfd = fd;
    listcode();
    write(outfd, linebuf, (unsigned) (lineptr - linebuf));
    writenl();
    if (errcount != 0)
	listerrors();
    listpre = TRUE;
    list_force=FALSE;
}

/* list object code for 1 line */

static void listcode(void)
{
    unsigned char count;
    struct code_listing_s *listptr;
    unsigned numlength;
    char *numptr;

    listptr = (struct code_listing_s *) temp_buf();
    memset((char *) listptr, ' ', sizeof *listptr);
    listptr->nullterm = 0;
    if (macflag)
    {
	listptr->linum_or_macro.macro.mark[0] = '+';
	listptr->linum_or_macro.macro.level[0] = maclevel + ('a' - 1);
    }
    else
    {
	numlength = LINUM_LEN;
	numptr = listptr->linum_or_macro.linum;
	if (infiln != infil0)
	{
	    *numptr++ = infiln - infil0 + ('a' - 1);
	    numlength = LINUM_LEN - 1;
	}
	build_number(linum, numlength, numptr);
    }
    if ((count = mcount) != 0 || popflags & POPLC)
	build_2hex_number((u16_T) lc, listptr->lc);
    if (popflags & POPLO)
    {
#if SIZEOF_OFFSET_T > 2
	if (popflags & POPLONG)
	    build_2hex_number((u16_T) (lastexp.offset / (offset_t) 0x10000L),
			      listptr->displ4);
#endif
	if (popflags & POPHI)
	    build_2hex_number((u16_T) lastexp.offset, listptr->displ2);
	else
	    build_1hex_number((opcode_pt) /* XXX */(u16_T) lastexp.offset, listptr->displ1);
	if (lastexp.data & RELBIT)
	    listptr->reldispl[0] = '>';
    }
    else if (count != 0)
    {
#ifdef I80386
	if (aprefix != 0)
	{
	    --count;
	    build_1hex_number(aprefix, listptr->aprefix);
	}
	if (oprefix != 0)
	{
	    --count;
	    build_1hex_number(oprefix, listptr->oprefix);
	}
	if (sprefix != 0)
	{
	    --count;
	    build_1hex_number(sprefix, listptr->sprefix);
	}
#endif
	if (page != 0)
	{
	    build_1hex_number(page, listptr->page);
	    --count;
	}
	build_1hex_number(opcode, listptr->opcode);
	--count;
	if (postb != 0)
	{
	    --count;
	    build_1hex_number(postb,
#ifdef MC6809
			      count == 0 ? listptr->displ1 :
#endif
			      listptr->postb);
	}
#ifdef I80386
	if (sib != NO_SIB)
	{
	    --count;
	    build_1hex_number(sib, listptr->sib);
	}
#endif
	if (count > 0)
	{
	    build_1hex_number((opcode_pt) lastexp.offset, listptr->displ1);
	    if (lastexp.data & RELBIT)
		listptr->reldispl[0] = '>';
	}
	if (count > 1)
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x8),
			      listptr->displ2);
#if SIZEOF_OFFSET_T > 2
	if (count > 2)
	{
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x10),
			      listptr->displ3);
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x18),
			      listptr->displ4);
	}
#endif
#ifdef I80386
	if (immcount > 0)
	{
	    build_1hex_number((opcode_pt) immadr.offset, listptr->imm1);
	    if (immadr.data & RELBIT)
		listptr->relimm[0] = '>';
	}
	if (immcount > 1)
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x8),
			      listptr->imm2);
	if (immcount > 2)
	{
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x10),
			      listptr->imm3);
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x18),
			      listptr->imm4);
	}
#endif
    }
    writes((char *) listptr);
}

/* list errors, assuming some */

static void listerrors(void)
{
    unsigned char column;
    unsigned char errcol;	/* column # in error line */
    unsigned char errcolw;	/* working column in error line */
    char *errmsg;
    struct error_s *errptr;
    char *linep;
    unsigned char remaining;

#ifdef I80386
    paderrorline(1);
#else
    paderrorline(CODE_LIST_LENGTH - LINUM_LEN);
#endif
    remaining = errcount;
    column = 0;			/* column to match with error column */
    errcolw = errcol = CODE_LIST_LENGTH; /* working & col number on err line */
    errptr = errbuf;
    linep = linebuf;
    do
    {
#ifdef I80386
        if(errcol != CODE_LIST_LENGTH)
	{
	    writenl(); paderrorline(1);
	}
	writes(errmsg = errptr->err_str);
	errcol = strlen(errmsg)+LINUM_LEN+1;
	column = 0; linep = linebuf;
        errcolw = CODE_LIST_LENGTH;
	while (errcolw > errcol)
	{
	    writec('.');
	    ++errcol;
	}
#endif
	while (errptr && errptr->position < 132 && column < errptr->position)
	{
	    ++column;
	    if (*linep++ == '\t')	/* next tab (standard tabs only) */
		errcolw = (errcolw + 8) & 0xf8;
	    else
		++errcolw;
	    while (errcolw > errcol)
	    {
#ifdef I80386
	        writec('.');
#else
		writec(' ');
#endif
		++errcol;
	    }
	}
#ifdef I80386
	writec('^'); ++errcol;
#else
	if (errcolw < errcol)	/* position under error on new line */
	{
	    writenl();
	    paderrorline((unsigned) errcolw - LINUM_LEN);
	}
	writec('^');
	writes(errmsg = errptr->err_str);
	errcol += strlen(errmsg);
#endif
	++errptr;
    }
    while (--remaining != 0);
    writenl();
    if (erroverflow)
    {
#ifdef I80386
	paderrorline(1);
#else
	paderrorline(CODE_LIST_LENGTH - LINUM_LEN);
#endif
	writesn(FURTHER);
    }
}

/* pad out error line to begin under 1st char of source listing */

static void paderrorline(unsigned nspaces)
{
    int nstars = LINUM_LEN;

    while (nstars-- != 0)
	writec('*');		/* stars under line number */
    while (nspaces-- != 0)
	writec(' ');		/* spaces out to error position */
}

/* write 1 character */

void writec(char ch)
{
    write(outfd, &ch, 1);
}

/* write newline */

void writenl(void)
{
    writes(SOS_EOLSTR);
}

/* write 1 offset_t, order to suit target */

void writeoff(offset_t offset)
{
    char buf[sizeof offset];

#if SIZEOF_OFFSET_T > 2
    u4c4(buf, offset);
#else
    u2c2(buf, offset);
#endif
    write(outfd, buf, sizeof buf);
}

/* write string */

void writes(const char *s)
{
    write(outfd, s, strlen(s));
}

/* write string followed by newline */

void writesn(const char *s)
{
    writes(s);
    writenl();
}

/* write 1 word, order to suit target */

void writew(unsigned word)
{
    char buf[2];

    u2c2(buf, (u16_T) word);
    write(outfd, buf, sizeof buf);
}
