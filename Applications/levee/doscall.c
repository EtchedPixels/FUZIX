/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-1997 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by David L Parsons (orc@pell.chi.il.us).  My name may not be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS PROVIDED
 * AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * dos interface for levee (Microsoft C)
 */
#include "levee.h"

#if MSDOS
#include <glob.h>
#include <dos.h>

int PROC
min(a,b)
int a,b;
{
    return (a>b) ? b : a;
}

int PROC
max(a,b)
int a,b;
{
    return (a<b) ? b : a;
}

PROC
strput(s)
char *s;
{
    write(1, s, strlen(s));
}

/* get a key, mapping certain control sequences
 */
PROC
getKey()
{
    register c;
    extern char _far crawcin();

    c = crawcin();

    if (c == 0 || c == 0xe0)
	switch (c=crawcin()) {
	case 'K': return LTARROW;
	case 'M': return RTARROW;
	case 'H': return UPARROW;
	case 'P': return DNARROW;
	case 'I': return 'U'-'@';	/* page-up */
	case 'Q': return 'D'-'@';	/* page-down */
	default : return 0;
	}
    return c;
}


/* don't allow interruptions to happen
 */
PROC
nointr()
{
    extern void _cdecl _interrupt _far ignore_ctrlc();
    _dos_setvect(0x23, ignore_ctrlc);
} /* nointr */


/* have ^C do what it usually does
 */
PROC
allowintr()
{
    extern void _cdecl _interrupt _far intr_on_ctrlc();
    _dos_setvect(0x23, intr_on_ctrlc);
} /* allowintr */


/*
 * basename() returns the filename part of a pathname
 */
char *
basename(s)
register char *s;
{
    register char *p = s;
    
    for (p = s+strlen(s); p > s; --p)
	if (p[-1] == '/' || p[-1] == '\\' || p[-1] == ':')
	    return p;
    return s;
} /* basename */


/*
 * glob() expands a wildcard, via calls to _dos_findfirst/_next()
 * and pathname retention.
 */
char *
glob(path, dta)
char *path;
struct glob_t *dta;
{
    static char path_bfr[256];		/* full pathname to return */
    static char *file_part;		/* points at file - for filling */
    static char isdotpattern;		/* looking for files starting with . */
    static char isdotordotdot;		/* special case . or .. */
    static struct glob_t *dta_bfr;	/* pointer to desired dta */
    static struct find_t dird;		/* DOS dta */

    register st;			/* status from _dos_findxxx */

    if (path) {
	/* when we start searching, save the path part of the filename in
	 * a safe place.
	 */
	strcpy(path_bfr, path);
	file_part = basename(path_bfr);

	/* set up initial parameters for DosFindFirst()
	 */
	dta_bfr = dta;
	
	if (isdotpattern = (*file_part == '.'))
	    /* _dos_findfirst() magically expands . and .. into their
	     * directory names.  Admittedly, there are cases where
	     * this can be useful, but this is not one of them. So,
	     * if we find that we're matching . and .., we just
	     * special-case ourselves into oblivion to get around
	     * this particular bit of DOS silliness.
	     */
	    isdotordotdot = (file_part[1] == 0 || file_part[1] == '.');
	else
	    isdotordotdot = 0;

	st = _dos_findfirst(path, 0x16, &dird);
    }
    else
	st = _dos_findnext(&dird);

    while (st == 0) {
	/* Unless the pattern has a leading ., don't include any file
	 * that starts with .
	 */
	if (dird.name[0] == '.' && !isdotpattern)
	    st = _dos_findnext(&dird);
	else {
	    /* found a file - affix the path leading to it, then return
	     * a pointer to the (static) buffer holding the path+the name.
	     */
	    strlwr(dird.name);		/* DOS & OS/2 are case-insensitive */

	    if (dta_bfr) {
		dta_bfr->wr_time = dird.wr_time;
		dta_bfr->wr_date = dird.wr_date;
		if (isdotordotdot)
		    strcpy(dta_bfr->name, file_part);
		else {
		    strncpy(dta_bfr->name, dird.name, sizeof(dta_bfr->name)-1);
		    dta_bfr->name[sizeof(dta_bfr->name)-1] = 0;
		}
		dta_bfr->size   = dird.size;
		dta_bfr->attrib = dird.attrib;
	    }
	    if (!isdotordotdot)
		strcpy(file_part, dird.name);
	    return path_bfr;
	}
    }
    /* nothing matched
     */
    if (path && isdotordotdot) {
	/* must be at root, so statting dot will most likely fail.  Fake a
	 * dta.
	 */
	if (dta_bfr) {
	    memset(dta_bfr, 0, sizeof *dta_bfr);
	    dta_bfr->attrib = 0x10;
	    dta_bfr->name[0] = '.';
	}
	return path_bfr;
    }
    return (char*)0;
} /* glob */
#endif /*MSDOS*/
