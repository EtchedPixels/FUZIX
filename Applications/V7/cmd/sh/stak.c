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

#include	"defs.h"

STKPTR stakbot = (void *)nullstr;



/* ========	storage allocation	======== */

STKPTR getstak(int asize)
{				/* allocate requested stack */
	register STKPTR oldstak;
	register int size;

	size = round(asize, BYTESPERWORD);
	oldstak = stakbot;
	staktop = stakbot += size;
	return oldstak;
}

STKPTR locstak(void)
{				/* set up stack for local use
				 * should be followed by `endstak'
				 */
	if (brkend - stakbot < BRKINCR) {
		setbrk(brkincr);
		if (brkincr < BRKMAX)
			brkincr += 256;
	}
	return stakbot;
}

STKPTR savstak(void)
{
	assert(staktop == stakbot);
	return stakbot;
}

STKPTR endstak(register char *argp)
{				/* tidy up after `locstak' */
	register char *oldstak;
	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (char *) round(argp, BYTESPERWORD);
	return oldstak;
}

void tdystak(register char *x)
{
	/* try to bring stack back to x */
	while (ADR(stakbsy) > ADR(x)) {
		sh_free(stakbsy);
		stakbsy = stakbsy->word;
	}
	staktop = stakbot = max(ADR(x), ADR(stakbas));
	rmtemp((IOPTR)x);
}

void stakchk(void)
{
	if ((brkend - stakbas) > BRKINCR + BRKINCR)
		setbrk(-BRKINCR);
}

char *cpystak(const char *x)
{
	return endstak(movstr(x, locstak()));
}
