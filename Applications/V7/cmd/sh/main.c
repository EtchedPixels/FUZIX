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
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	"sym.h"
#include	"timeout.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<setjmp.h>
#include	<readline/readline.h>

UFD output = 2;
static BOOL beenhere = FALSE;
CHAR tmpout[20] = "/tmp/sh-";
FILEBLK stdfile;
FILE standin = &stdfile;
char* tempfile;

static void exfile(BOOL);

#ifdef BUILD_FSH
static char history[1024];
static char inbuf[256];
static unsigned int inleft;
static char *inptr;
static char ineof;

static int line_input(const char *prmpt)
{
	register int l;
	if (!isatty(standin->fdes))
		return -1;	/* Not a tty */
	do {
		l = rl_edit(standin->fdes, output,
			prmpt,
			inbuf, 256);
		if (l >= 0) {
			inbuf[l] = '\n';
			inptr = inbuf;
			inleft = l + 1;
			ineof = 0;
		}
		else
			ineof = 1;
	} while(l == -2);
	/* 0 - EOF, 1+ buffer including \n */
	return l + 1;
}

int lineread(int fd, char *bufp, int len)
{
	register char *buf = bufp;
	int bias = 0;
	int r;
	if (fd == standin->fdes && inleft) {
		if (len <= inleft) {
			memcpy(buf, inptr, len);
			inleft -= len;
			inptr += len;
			return len;
		}
		memcpy(buf, inptr, inleft);
		len -= inleft;
		buf += inleft;
		bias = inleft;
		inleft = 0;
	}
	r = 0;
	if (!ineof)
		r = read(fd, buf, len);
	if (r >= 0)
		r += bias;
	return r;
}

#else
#define rl_hinit(x,y)
#define line_input(x)	(-1)
#endif

int main(int c, const char *v[])
{
	register int rflag = ttyflg;

	/* initialise storage allocation */
	blokinit();

	stdsigs();

	setbrk(BRKINCR);
	addblok((POS) 0);

	rl_hinit(history, sizeof(history));

	/* set names from userenv */
	/* sh_getenv can call error handlers so initialize the
	   subshell trap and if it fails (eg being passed a broken
	   environment) just carry on instead of entering hyperspace */
	if (setjmp(subshell) == 0)
		sh_getenv();

	/* look for restricted */
/*	if(c>0 && any('r', *v) ) { rflag=0 ;} */

	/* look for options */
	dolc = options(c, v);
	if (dolc < 2)
		flags |= stdflg;

	if ((flags & stdflg) == 0)
		dolc--;

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
			if (line_input(ps1nod.namval) < 0)
			{
				prs(ps1nod.namval);
				alarm(TIMEOUT);
			}
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
	if ((flags & prompt) && standin->fstak == 0 && eor == NL) {
		if (line_input(ps2nod.namval) < 0)
			prs(ps2nod.namval);
	}
}

void settmp(void)
{
	itos(getpid());
	serial = 0;
	tempfile = movstr(numbuf, &tmpout[TMPNAM]);
}

void Ldup(register int fa, register int fb)
{
	dup2(fa, fb);
	close(fa);
	fcntl(fb, F_SETFD, FD_CLOEXEC);
}
