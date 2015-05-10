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
	return (oldstak);
}

STKPTR locstak(void)
{				/* set up stack for local use
				 * should be followed by `endstak'
				 */
	if (brkend - stakbot < BRKINCR) {
		setbrk(brkincr);
		if (brkincr < BRKMAX) {
			brkincr += 256;
			;
		};
	}
	return (stakbot);
}

STKPTR savstak(void)
{
        /* FIXME: check assert doesn't suck in stdio */
	assert(staktop == stakbot);
	return (stakbot);
}

STKPTR endstak(register STRING argp)
{				/* tidy up after `locstak' */
	register STKPTR oldstak;
	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (STKPTR) round(argp, BYTESPERWORD);
	return (oldstak);
}

void tdystak(register STKPTR x)
{
	/* try to bring stack back to x */
	while (ADR(stakbsy) > ADR(x)
	    ) {
		free(stakbsy);
		stakbsy = stakbsy->word;
		;
	}
	staktop = stakbot = max(ADR(x), ADR(stakbas));
	rmtemp((const char *)x);
}

void stakchk(void)
{
	if ((brkend - stakbas) > BRKINCR + BRKINCR) {
		setbrk(-BRKINCR);
	}
}

STKPTR cpystak(STKPTR x)
{
	return (endstak(movstr(x, locstak())));
}
