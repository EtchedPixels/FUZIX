/* macro.c - expand macros for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "globvar.h"
#include "scan.h"
#undef EXTERN
#define EXTERN
#include "macro.h"

/*
  Enter macro: stack macro and get its parameters.
  Parameters form a linked list of null-terminated strings of form
  next:string. The first string is the macro number in 4 bytes.
*/

void entermac(struct sym_s *symptr)
{
    if (maclevel >= MAXMAC)
	error(MACOV);
    else if (macpar + 2 > macptop)
	error(PAROV);		/* no room for 0th param */
				/* (2 structs to fit it!) */
    else
    {
	char ch;
	struct schain_s *param1;
	register char *reglineptr;
	register char *stringptr;

	++maclevel;
	(--macstak)->text = (char *) symptr->value_reg_or_op.value;
	macstak->parameters = param1 = macpar;
	param1->next = NUL_PTR;
	*(stringptr = build_number(++macnum, 3, param1->string)) = 0;
	macpar = (struct schain_s *) (stringptr + 1);
				/* TODO: alignment */
	getsym();
	if (sym == EOLSYM)
	    return;		/* no other params */
	if (sym != LPAREN)
	    reglineptr = symname;
	else
	    reglineptr = lineptr;
	stringptr = macpar->string;
	while (TRUE)
	{
	    if (stringptr >= (char *) macptop)
	    {
		symname = reglineptr;
		error(PAROV);
		return;
	    }
	    ch = *reglineptr++;
	    if (ch == '\\')
		/* escaped means no special meaning for slash, comma, paren */
		ch = *reglineptr++;
	    else if (ch == ',' || ch == ')' || ch == '!' || ch == ';' 
		  || ch == '\n' || ch == 0)
	    {
		if (stringptr >= (char *) macptop)
		{
		    symname = reglineptr;
		    error(PAROV);	/* no room for null */
		    return;
		}
		*stringptr = 0;
		param1->next = macpar;	/* ptr from previous */
		(param1 = macpar)->next = NUL_PTR;
					/* this goes nowhere */
		macpar = (struct schain_s *) (stringptr + 1);
					/* but is finished OK - TODO align */
		stringptr = macpar->string;
		if (ch != ',')
		    return;
		continue;
	    }
	    if ((*stringptr++ = ch) == 0)
	    {
		symname = reglineptr;
		error(RPEXP);
		return;
	    }
	}
    }
}

/* MACRO pseudo-op */

void pmacro(void)
{
    bool_t saving;
    bool_t savingc;
    struct sym_s *symptr=0;
    int    maclen = 8;
    int    macoff = 0;
    char * macbuf = asalloc(8);

    saving =			/* prepare for bad macro */
	savingc = FALSE;	/* normally don't save comments */
    macload = TRUE;		/* show loading */
    if (label != NUL_PTR)
	error(ILLAB);
    else if (sym != IDENT)
	error(LABEXP);
    else
    {
	symptr = gsymptr;
	if (symptr->type & MNREGBIT)
	    error(LABEXP);
	else if (symptr->type & LABIT || symptr->data & FORBIT)
	    error(RELAB);
	else if (pass != last_pass || symptr->type & REDBIT)
				/* copy on pass 0, also pass 1 if redefined */
	{
	    saving = TRUE;
	    if (symptr->type & MACBIT)
		symptr->type |= REDBIT;
	    else
		symptr->type |= MACBIT;
	    symptr->data = UNDBIT;	/* undefined till end */
	    symptr->value_reg_or_op.value = (offset_t) macbuf;
	    getsym_nolookup();		/* test for "C" */
	    if (sym == IDENT && lineptr == symname + 1 && *symname == 'C')
		savingc = TRUE;
	}
    }
    while (TRUE)
    {
	skipline();
	listline();
	readline();
	if (!macload)
	    break;		/* macload cleared to show eof */
	getsym_nolookup();
	if (sym == IDENT)
	{
	    if (lineptr == symname + 4 &&
	        ( strncmp(symname, "MEND", 4) == 0 || strncmp(symname, "mend", 4) == 0) )
            {
                getsym();
		break;
            }
	}
	else if (sym != MACROARG)
	{
	    if (!savingc)
		continue;	/* don't save comment */
	}
	if (!saving)
	    continue;
	{
	    char * p = strchr(linebuf, EOLCHAR);
	    int len = (p-linebuf+1);

	    if ( macoff + len > maclen-4 )
	    {
	        maclen = maclen * 2 + len;
	        macbuf = asrealloc(macbuf, maclen);
	    }
	    memcpy(macbuf+macoff, linebuf, len);
	    macoff += len;

	}
    }
    macload = FALSE;
    if (saving)
    {
	macbuf[macoff] = ETB;
        symptr->value_reg_or_op.value = (offset_t) macbuf;
	symptr->data = 0;
    }
}
