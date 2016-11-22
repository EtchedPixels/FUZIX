/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

static const char **copyargs(const char *from[], int n);
static DOLPTR dolh;

CHAR flagadr[10];

CHAR flagchar[] = {
	'x', 'n', 'v', 't', 's', 'i', 'e', 'r', 'k', 'u', 0
};

int flagval[] = {
	execpr, noexec, readpr, oneflg, stdflg, intflg, errflg, rshflg,
	    keyflg, setflg, 0
};

/* ========	option handling	======== */


int options(int argc, const char *argv[])
{
	register const char *cp;
	register const char **argp = argv;
	register char *flagc;
	char *flagp;

	if (argc > 1 && *argp[1] == '-') {
		cp = argp[1];
		flags &= ~(execpr | readpr);
		while (*++cp) {
			flagc = flagchar;

			while (*flagc && *flagc != *cp) {
				flagc++;
			}
			if (*cp == *flagc) {
				flags |= flagval[flagc - flagchar];
			} else if (*cp == 'c' && argc > 2 && comdiv == 0) {
				comdiv = argp[2];
				argp[1] = argp[0];
				argp++;
				argc--;
			} else {
				failed(argv[1], badopt);
			}
		}
		argp[1] = argp[0];
		argc--;
	}

	/* set up $- */
	flagc = flagchar;
	flagp = flagadr;
	while (*flagc) {
		if (flags & flagval[flagc - flagchar]) {
			*flagp++ = *flagc;
		}
		flagc++;
	}
	*flagp++ = 0;
	return argc;
}

void setargs(const char *argi[])
{
	/* count args */
	register const char **argp = argi;
	register int argn = 0;

	while (((intptr_t)(*argp++)) != ENDARGS)
		argn++;

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh = (DOLPTR) copyargs(argi, argn);	/* sets dolv */
	assnum(&dolladr, dolc = argn - 1);
}

DOLPTR freeargs(DOLPTR blk)
{
	register char **argp;
	register DOLPTR argr = 0;
	register DOLPTR argblk;

	if ( (argblk = blk) ) {
		argr = argblk->dolnxt;
		if ((--argblk->doluse) == 0) {
			for (argp = (char **) argblk->dolarg;
			     ((intptr_t)(*argp)) != ENDARGS; argp++) {
				sh_free(*argp);
			}
			sh_free(argblk);
		}
	}
	return (argr);
}

static const char **copyargs(const char *from[], int n)
{
	register const char **np =
	    (const char **) alloc(sizeof(char *) * n + 3 * BYTESPERWORD);
	register const char **fp = from;
	register const char **pp = np;

	((DOLPTR) np)->doluse = 1;	/* use count */
	np = (const char **) ((DOLPTR) np)->dolarg;
	dolv = np;

	while (n--) {
		*np++ = make(*fp++);
	}
	*np++ = ENDARGS;
	return pp;
}

void clearup(void)
{
	/* force `for' $* lists to go away */
	while ( (argfor = freeargs(argfor)) );

	/* clean up io files */
	while (pop());
}

DOLPTR useargs(void)
{
	if (dolh) {
		dolh->doluse++;
		dolh->dolnxt = argfor;
		return (argfor = dolh);
	} else {
		return (0);
		;
	}
}
