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


STRING		trapcom[MAXTRAP];
BOOL		trapflg[MAXTRAP];

/* ========	fault handling routines	   ======== */


void	fault(sig)
	REG INT		sig;
{
	REG INT		flag;

	signal(sig,fault);
	IF sig==MEMF
	) {	IF setbrk(brkincr) == -1
		) {	error(nospace);
		FI
	} else if (sig==ALARM
	) {	IF flags&waiting
		) {	done();
		FI
	} else {	flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
	FI
}

stdsigs()
{
	ignsig(QUIT);
	getsig(INTR);
	getsig(MEMF);
	getsig(ALARM);
}

ignsig(n)
{
	REG INT		s, i;
#if 0
    // FIXME: need to do proper SIG_IGN checks/handling
	IF (s=signal(i=n,1)&01)==0
	) {	trapflg[i] |= SIGMOD;
	FI
#endif	
	return(s);
}

getsig(n)
{
	REG INT		i;

	IF trapflg[i=n]&SIGMOD || ignsig(i)==0
	) {	signal(i,fault);
	FI
}

oldsigs()
{
	REG INT		i;
	REG STRING	t;

	i=MAXTRAP;
	WHILE i--
	DO  t=trapcom[i];
	    IF t==0 || *t
	    ) { clrsig(i);
	    FI
	    trapflg[i]=0;
	OD
	trapnote=0;
}

clrsig(i)
	INT		i;
{
	free(trapcom[i]); trapcom[i]=0;
	IF trapflg[i]&SIGMOD
	) {	signal(i,fault);
		trapflg[i] &= ~SIGMOD;
	FI
}

chktrap()
{
	/* check for traps */
	REG INT		i=MAXTRAP;
	REG STRING	t;

	trapnote &= ~TRAPSET;
	WHILE --i
	DO IF trapflg[i]&TRAPSET
	   ) { trapflg[i] &= ~TRAPSET;
		IF t=trapcom[i]
		) {	INT	savxit=exitval;
			execexp(t,0);
			exitval=savxit; exitset();
		FI
	   FI
	OD
}
