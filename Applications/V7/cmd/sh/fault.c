/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"


STRING trapcom[MAXTRAP];
BOOL trapflg[MAXTRAP];

/* ========	fault handling routines	   ======== */


void fault(register int sig)
{
	register int flag;

	signal(sig, fault);
	if (sig == MEMF) {
		if (setbrk(brkincr) == (void *)-1)
			error(nospace);
	} else if (sig == ALARM) {
		if (flags & waiting)
			done();
	} else {
		flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
	}
}

void stdsigs(void)
{
	ignsig(QUIT);
	getsig(INTR);
	getsig(MEMF);
	getsig(ALARM);
}

sighandler_t ignsig(int n)
{
	register int i;
	sighandler_t s;
	/* FIXME: Was a test of the low bit.. not clear this is the correct translation of V7 internals */
	if ((s = signal(i = n, SIG_IGN)) != SIG_IGN)
		trapflg[i] |= SIGMOD;
	return s;
}

void getsig(int n)
{
	register int i;

	/* Again was a zero test for ignsig, unclear if correct translation FIXME */
	if (trapflg[i = n] & SIGMOD || ignsig(i) == SIG_DFL)
		signal(i, fault);
}

void oldsigs(void)
{
	register int i;
	register char *t;

	i = MAXTRAP;
	while (i--) {
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

void clrsig(int i)
{
	sh_free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD) {
		signal(i, fault);
		trapflg[i] &= ~SIGMOD;
	}
}

void chktrap(void)
{
	/* check for traps */
	register int i = MAXTRAP;
	register STRING t;

	trapnote &= ~TRAPSET;
	while (--i) {
		if (trapflg[i] & TRAPSET) {
			trapflg[i] &= ~TRAPSET;
			if ( (t = trapcom[i]) ) {
				int savxit = exitval;
				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}
