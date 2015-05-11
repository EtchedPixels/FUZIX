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

#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	"defs.h"
#include	"dup.h"
#include	"sym.h"
#include	"timeout.h"
#include	<sys/types.h>
#include	<sys/stat.h>

UFD output = 2;
static BOOL beenhere = FALSE;
CHAR tmpout[20] = "/tmp/sh-";
FILEBLK stdfile;
FILE standin = &stdfile;

static void exfile(BOOL);

int main(int c, const char *v[])
{
	register int rflag = ttyflg;

	/* initialise storage allocation */
	stdsigs();
	setbrk(BRKINCR);
	addblok((POS) 0);

	/* set names from userenv */
	sh_getenv();

	/* look for restricted */
/*	if(c>0 && any('r', *v) ) { rflag=0 ;} */

	/* look for options */
	dolc = options(c, v);
	if (dolc < 2) {
		flags |= stdflg;
	}
	if ((flags & stdflg) == 0) {
		dolc--;
		;
	}
	dolv = v + c - dolc;
	dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr, dolc);
	cmdadr = (char *)dolv[0];

	/* set pidname */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default ifs */
	dfault(&ifsnod, sptbnl);

	if ((beenhere++) == FALSE) {	/* ? profile */
		if (*cmdadr == '-'
		    && (input = pathopen(nullstr, profile)) >= 0) {
			exfile(rflag);
			flags &= ~ttyflg;
			;
		}
		if (rflag == 0) {
			flags |= rshflg;
		}

		/* open input file if specified */
		if (comdiv) {
			estabf(comdiv);
			input = -1;
		} else {
			input = ((flags & stdflg) ? 0 : chkopen(cmdadr));
			comdiv--;
			;
		}
//      } else {        *execargs=(char *)dolv; /* for `ps' cmd */
		;
	}

	exfile(0);
	done();
}

static void exfile(BOOL prof)
{
	register L_INT mailtime = 0;
	register int userid;
	struct stat statb;

	/* move input */
	if (input > 0) {
		Ldup(input, INIO);
		input = INIO;
	}

	/* move output to safe place */
	if (output == 2) {
		Ldup(dup(2), OTIO);
		output = OTIO;
	}

	userid = getuid();

	/* decide whether interactive */
	if ((flags & intflg)
	    || ((flags & oneflg) == 0 && isatty(output) && isatty(input))) {
		dfault(&ps1nod, (userid ? stdprompt : supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg | prompt;
		ignsig(KILL);
	} else {
		flags |= prof;
		flags &= ~prompt;
	}

	if (setjmp(errshell) && prof) {
		close(input);
		return;
	}

	/* error return here */
	loopcnt = breakcnt = peekc = 0;
	iopend = 0;
	if (input >= 0)
		initf(input);

	/* command loop */
	for (;;) {
		tdystak(0);
		stakchk();	/* may reduce sbrk */
		exitset();
		if ((flags & prompt) && standin->fstak == 0 && !eof) {
			if (mailnod.namval
			    && stat(mailnod.namval, &statb) >= 0
			    && statb.st_size
			    && (statb.st_mtime != mailtime)
			    && mailtime) {
				prs(mailmsg);
			}
			mailtime = statb.st_mtime;
			prs(ps1nod.namval);
			alarm(TIMEOUT);
			flags |= waiting;
		}

		trapnote = 0;
		peekc = readc();
		if (eof)
			return;
		alarm(0);
		flags &= ~waiting;
		execute(cmd(NL, MTFLG), 0, NULL, NULL);
		eof |= (flags & oneflg);
	}
}

void chkpr(char eor)
{
	if ((flags & prompt) && standin->fstak == 0 && eor == NL)
		prs(ps2nod.namval);
}

void settmp(void)
{
	itos(getpid());
	serial = 0;
	tmpnam = movstr(numbuf, &tmpout[TMPNAM]);
}

void Ldup(register int fa, register int fb)
{
	dup2(fa, fb);
	close(fa);
	fcntl(fb, F_SETFD, FD_CLOEXEC);
}
