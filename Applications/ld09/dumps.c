/* dumps.c - print data about symbols and modules for linker */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "const.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

/* print list of modules and whether they are loaded */

void dumpmods(void)
{
    struct modstruct *modptr;
    char *s, *d;
    int i;

    for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext)
    {
	for(s=d=modptr->filename; *s ; s++)
	   if( *s == '/' ) d=s+1;
        if( memcmp(d, "libc", 4) == 0 && !modptr->loadflag ) continue;

	putstr(modptr->modname);
	i = strlen(modptr->modname);
	while(i<16) putbyte(' '),i++;
	putbyte( modptr->loadflag ? '+':'-' );
	putstr(d);
	if( modptr->archentry )
	{
	   putbyte('(');
	   putstr(modptr->archentry);
	   putbyte(')');
	}
	putbyte('\n');
    }
}

/* print data about symbols (in loaded modules only) */

void dumpsyms(void)
{
    flags_t flags;
    struct modstruct *modptr;
    struct symstruct **symparray;
    struct symstruct *symptr;
    char uflag;

    for (modptr = modfirst; modptr != NUL_PTR; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != NUL_PTR; ++symparray)
		if (symptr->modptr == modptr)
		{
		    uflag = FALSE;
		    if (((flags = symptr->flags) & (C_MASK | I_MASK)) == I_MASK)
			uflag = TRUE;
		    putbstr(20, uflag ? "" : modptr->modname);
		    putstr("  ");
		    putbstr(20, symptr->name);
		    putstr("  ");
		    putbyte(hexdigit[flags & SEGM_MASK]);
		    putstr("  ");
		    if (uflag)
			putstr("        ");
		    else
#ifdef LONG_OFFSETS
			put08lx(symptr->value);
#else
			put08x(symptr->value);
#endif
		    if( flags & (E_MASK|C_MASK) )
		       putstr(flags & A_MASK ? "  A" : "  R");
		    else
		       putstr(flags & A_MASK ? "  a" : "  r");
		    if (uflag)
			putstr(" U");
		    if (flags & C_MASK)
			putstr(" C");
		    if (flags & N_MASK)
			putstr(" N");
		    putbyte('\n');
		}
	}

    putstr("Total memory used: ");
#ifdef LONG_OFFSETS
    put08lx(memory_used());
#else
    put08x(memory_used());
#endif
    putbyte('\n');
}
