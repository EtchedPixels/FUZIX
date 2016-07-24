/* express.c - expression handler for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "address.h"
#include "globvar.h"
#include "scan.h"
#include "source.h"

static void experror(char * err_str);
static void expundefined(void);
static void simple2(void);
static void simple(void);
static void term(void);
static void factor2(void);

void absexpres(void)
{
    expres();
    chkabs();
}

/* check lastexp.data is abs */

void chkabs(void)
{
    if (lastexp.data & RELBIT)
    {
	if (pass == last_pass)
	    error(ABSREQ);
	expundefined();
    }
}

void experror(char *err_str)
{
    error(err_str);
    expundefined();
}

void expundefined(void)
{
    if( last_pass == 1 )
       lastexp.data = FORBIT | UNDBIT;
    else
       lastexp.data = UNDBIT;
}

void nonimpexpres(void)
{
    expres();
    if (lastexp.data & IMPBIT)
	experror(NONIMPREQ);
}

/* generate relocation error if pass 2, make lastexp.data forward&undefined */

void showrelbad(void)
{
    if (pass == last_pass)
	error(RELBAD);
    expundefined();
}

void symabsexpres(void)
{
    getsym();
    absexpres();
}

void symexpres(void)
{
    getsym();
    expres();
}

/*
  expres() parses expression = simple expression [op simple expression],
  where op is =, < or >.
  Parameters: sym, number in number, identifier from symname to lineptr - 1.
  Returns value in lastexp.
*/

void expres(void)
{
    offset_t leftoffset;

    simple();
    leftoffset = lastexp.offset;
    if (sym == EQOP)
    {
	simple2();
	if (leftoffset == lastexp.offset)
	    lastexp.offset = -1;
	else
	    lastexp.offset = 0;
    }
    else if (sym == LESSTHAN)
    {
	/* context-sensitive, LESSTHAN really means less than here */
	simple2();
	if (leftoffset < lastexp.offset)
	    lastexp.offset = -1;
	else
	    lastexp.offset = 0;
    }
    else if (sym == GREATERTHAN)
    {
	/* context-sensitive, GREATERTHAN really means greater than here */
	simple2();
	if (leftoffset > lastexp.offset)
	    lastexp.offset = -1;
	else
	    lastexp.offset = 0;
    }
}

/* get symbol and 2nd simple expression, check both rel or both abs */

void simple2(void)
{
    unsigned char leftdata;

    leftdata = lastexp.data;
    getsym();
    simple();
    if ((leftdata | lastexp.data) & IMPBIT ||
	(leftdata ^ lastexp.data) & (RELBIT | SEGM))
	showrelbad();
    else
	lastexp.data = (leftdata & lastexp.data) & ~(RELBIT | SEGM);
}

/*
  simple() parses simple expression = [+-] term {op term},
  where op is +, -, or \ (OR).
*/

void simple(void)
{
    offset_t leftoffset;
    unsigned char leftdata;

    if (sym == ADDOP || sym == SUBOP)
	lastexp.data = lastexp.offset = 0;
    else
	term();
    while (TRUE)
    {
	leftoffset = lastexp.offset;
	leftdata = lastexp.data;
	if (sym == ADDOP)
	{
	    getsym();
	    term();
	    if (leftdata & lastexp.data & RELBIT)
		showrelbad();	/* rel + rel no good */
	    else
		lastexp.data |= leftdata;
	    lastexp.offset += leftoffset;
	}
	else if (sym == SUBOP)
	{
	    getsym();
	    term();
	    /* check not abs - rel or rel - rel with mismatch */
	    if (lastexp.data & RELBIT &&
		(!(leftdata & RELBIT) ||
		 (leftdata | lastexp.data) & IMPBIT ||
		 (leftdata ^ lastexp.data) & (RELBIT | SEGM)))
		showrelbad();
	    else
		lastexp.data = ((leftdata | lastexp.data) & ~(RELBIT | SEGM))
			     | ((leftdata ^ lastexp.data) &  (RELBIT | SEGM));
	    lastexp.offset = leftoffset - lastexp.offset;
	}
	else if (sym == OROP)
	{
	    getsym();
	    term();
	    lastexp.data |= leftdata;
	    chkabs();		/* both must be absolute */
	    lastexp.offset |= leftoffset;
	}
	else
	    return;
    }
}

/* term() parses term = factor {op factor}, where op is *, /, &, <<, or >>. */

void term(void)
{
    offset_t leftoffset;

    factor();
    while (TRUE)
    {
	leftoffset = lastexp.offset;
	if (sym == STAR)
	{
	    /* context-sensitive, STAR means multiplication here */
	    factor2();
	    lastexp.offset *= leftoffset;
	}
	else if (sym == SLASH)
	{
	    /* context-sensitive, SLASH means division here */
	    factor2();
	    lastexp.offset = leftoffset / lastexp.offset;
	}
	else if (sym == ANDOP)
	{
	    factor2();
	    lastexp.offset &= leftoffset;
	}
	else if (sym == SLOP)
	{
	    factor2();
	    lastexp.offset = leftoffset << lastexp.offset;
	}
	else if (sym == SROP)
	{
	    factor2();
	    lastexp.offset = leftoffset >> lastexp.offset;
	}
	else
	    return;
    }
}

/* get symbol and 2nd or later factor, check both abs */

void factor2(void)
{
    unsigned char leftdata;

    leftdata = lastexp.data;
    getsym();
    factor();
    lastexp.data |= leftdata;
    chkabs();
}

/*
  factor() parses factor = number | identifier | * | (expression) | ! factor,
  ! is complementation. Returns value in lastexp.offset, possible flags
  IMPBIT, FORBIT, RELBIT and UNDBIT in lastexp.data, and segment in SEGM
  part of lastexp.data, and lastexp.sym at imported symbol if IMPBIT.
  If the factor is an identifier, LOOKUP is used to get its value
  (so the ident is installed in the symbol table if necessary, with
  default flags inidata). If the identifier is not a label,
  (could be imported, or later in the program), its FORBIT is set.
  The expression FORBIT, IMPBIT, RELBIT, UNDBIT and SEGM are then
  taken from the identifier.
*/

void factor(void)
{
    switch (sym)
    {
    case SLASH:
	/* context-sensitive, SLASH means a hex number here */
	context_hexconst();
    case INTCONST:
	lastexp.data = 0;	/* absolute & not forward or undefined */
	lastexp.offset = number;
	getsym();
	return;
    case IDENT:
	{
	    register struct sym_s *symptr;

	    symptr = gsymptr;
	    if (symptr->type & (MNREGBIT | MACBIT))
		experror(symptr->type & MACBIT ? MACUID :
			 symptr->data & REGBIT ? REGUID : MNUID);
	    else
	    {
		if (!(symptr->type & (LABIT | VARBIT)))
		{
                    if( last_pass == 1 )
		        symptr->data |= FORBIT;
		    lastexp.sym = symptr;
		}
		if (pass != last_pass)
		{
                    if( last_pass == 1 )
		        lastexp.data = symptr->data &
			    (FORBIT | RELBIT | UNDBIT | SEGM);
		    else
		        lastexp.data = symptr->data &
			    (RELBIT | UNDBIT | SEGM);
				/* possible flags for pass 1 */
		    lastexp.offset = symptr->value_reg_or_op.value;
		}
		else
		{
		    if ((lastexp.data = symptr->data) & IMPBIT)
			lastexp.offset = 0;	/* value != 0 for commons */
						/* OK even if UNDBIT */
		    else
		    {
			lastexp.offset = symptr->value_reg_or_op.value;
			if (lastexp.data & UNDBIT)
			    experror(UNBLAB);
		    }
		}
	    }
	    getsym();
	    return;
	}
#ifndef MC6809
    case LBRACKET:
	if (!asld_compatible)
	    break;		/* error, LPAREN is the grouping symbol */
	getsym();
	expres();
	if (sym != RBRACKET)
	    error(RBEXP);
	else
	    getsym();
	return;
#endif
    case LPAREN:
#ifndef MC6809
	if (asld_compatible)
	    break;		/* error, LBRACKET is the grouping symbol */
#endif
	getsym();
	expres();
	if (sym != RPAREN)
	    error(RPEXP);
	else
	    getsym();
	return;
    case NOTOP:
	getsym();
	factor();
	chkabs();
	lastexp.offset = ~lastexp.offset;
	return;
    case ADDOP:
	getsym();
	factor();
	return;
    case SUBOP:
	getsym();
	factor();
	chkabs();
	lastexp.offset = -lastexp.offset;
	return;
    case STAR:
	/* context-sensitive, STAR means location counter here */
	lastexp.offset = lc;
	if ((lastexp.data = lcdata) & UNDBIT && pass == last_pass)
	    experror(UNBLAB);
	getsym();
	return;
    }
    experror(FACEXP);
}

/*
  string compare for IFC/ELSEIFC
  expects (<string1>,<string2>)
  returns logical value in lastexp
*/

void scompare(void)
{
    /* prepare flags for OK, lastexp.offset for error */
    lastexp.data = lastexp.offset = 0;
    if (sym != LPAREN)
	experror(LPEXP);
    else
    {
	register char *string1;
	register char *string2;

	for (string2 = string1 = lineptr; *string2 != ','; ++string2)
	    if (*string2 == 0 || *string2 == ')')
	    {
		symname = string2;
		experror(COMEXP);
		return;
	    }
	string2++;
	while (*string1++ == *string2++)
	    ;
	if (string2[-1] == ')')
	{
	    if (string1[-1] == ',')
		lastexp.offset = TRUE;	/* else leave FALSE */
	    lineptr = string2;
	}
	else			/* FALSE, keep reading to verify syntax */
	{
	    for (; *string2 != ')'; ++string2)
		if (*string2 == 0 || *string2 == ',')
		{
		    symname = string2;
		    experror(RPEXP);
		}
	    lineptr = ++string2;
	}
	getsym();
    }
}
