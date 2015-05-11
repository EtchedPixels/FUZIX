/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	<stdlib.h>
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

int ignsig(int n)
{
	register int s, i;
#if 0
	// FIXME: need to do proper SIG_IGN checks/handling
	if ((s = signal(i = n, 1) & 01) == 0) {
		trapflg[i] |= SIGMOD;
	}
#endif
	return (s);
}

void getsig(int n)
{
	register int i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0) {
		signal(i, fault);
		;
	}
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
			if (t = trapcom[i]) {
				int savxit = exitval;
				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}
