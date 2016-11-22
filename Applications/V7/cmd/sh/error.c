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


/* ========	error handling	======== */

void exitset(void)
{
	assnum(&exitadr, exitval);
}

void sigchk(void)
{
	/* Find out if it is time to go away.
	 * `trapnote' is set to SIGSET when fault is seen and
	 * no trap has been set.
	 */
	if (trapnote & SIGSET)
		exitsh(SIGFAIL);
}

void failed(const char *s1, const char *s2)
{
	prp();
	prs(s1);
	if (s2) {
		prs(colon);
		prs(s2);
		;
	}
	newline();
	exitsh(ERROR);
}

void error(const char *s)
{
	failed(s, NIL);
}

void exitsh(int xno)
{
	/* Arrive here from `FATAL' errors
	 *  a) exit command,
	 *  b) default trap,
	 *  c) fault with no trap set.
	 *
	 * Action is to return to command level or exit.
	 */
	exitval = xno;
	if ((flags & (forked | errflg | ttyflg)) != ttyflg) {
		done();
	} else {
		clearup();
		longjmp(errshell, 1);
		;
	}
}

void done(void)
{
	register STRING t;
	if ( (t = trapcom[0]) ) {
		trapcom[0] = 0;	/*should free but not long */
		execexp(t, 0);
		;
	}
	rmtemp(0);
	exit(exitval);
}

void rmtemp(IOPTR base)
{
	while (iotemp > base) {
		unlink(iotemp->ioname);
		iotemp = iotemp->iolst;
	}
}
