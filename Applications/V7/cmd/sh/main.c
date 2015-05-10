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

UFD		output = 2;
static BOOL	beenhere = FALSE;
CHAR		tmpout[20] = "/tmp/sh-";
FILEBLK		stdfile;
FILE		standin = &stdfile;

static void	exfile();




main(c, v)
	INT		c;
	STRING		v[];
{
	REG INT		rflag=ttyflg;

	/* initialise storage allocation */
	stdsigs();
	setbrk(BRKINCR);
	addblok((POS)0);

	/* set names from userenv */
	sh_getenv();

	/* look for restricted */
/*	IF c>0 && any('r', *v) ) { rflag=0 FI */

	/* look for options */
	dolc=options(c,v);
	IF dolc<2 ) { flags |= stdflg FI
	IF (flags&stdflg)==0
	) {	dolc--;
	FI
	dolv=v+c-dolc; dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr,dolc);
	cmdadr=dolv[0];

	/* set pidname */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default ifs */
	dfault(&ifsnod, sptbnl);

	IF (beenhere++)==FALSE
	) {	/* ? profile */
		IF *cmdadr=='-'
		    && (input=pathopen(nullstr, profile))>=0
		) {	exfile(rflag); flags &= ~ttyflg;
		FI
		IF rflag==0 ) { flags |= rshflg FI

		/* open input file if specified */
		IF comdiv
		) {	estabf(comdiv); input = -1;
		} else {	input=((flags&stdflg) ? 0 : chkopen(cmdadr));
			comdiv--;
		FI
//	} else {	*execargs=(char *)dolv;	/* for `ps' cmd */
	FI

	exfile(0);
	done();
}

static void	exfile(prof)
BOOL		prof;
{
	REG L_INT	mailtime = 0;
	REG INT		userid;
	struct stat	statb;

	/* move input */
	IF input>0
	) {	Ldup(input,INIO);
		input=INIO;
	FI

	/* move output to safe place */
	IF output==2
	) {	Ldup(dup(2),OTIO);
		output=OTIO;
	FI

	userid=getuid();

	/* decide whether interactive */
	IF (flags&intflg) || ((flags&oneflg)==0 && isatty(output) && isatty(input))
	) {	dfault(&ps1nod, (userid?stdprompt:supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg|prompt; ignsig(KILL);
	} else {
		flags |= prof; flags &= ~prompt;
	FI

	IF setjmp(errshell) && prof
	) {	close(input); return;
	FI

	/* error return here */
	loopcnt=breakcnt=peekc=0; iopend=0;
	IF input>=0 ) { initf(input) FI

	/* command loop */
	for(;;) {
		tdystak(0);
		stakchk(); /* may reduce sbrk */
		exitset();
		IF (flags&prompt) && standin->fstak==0 && !eof
		) {	IF mailnod.namval
			    && stat(mailnod.namval,&statb)>=0 && statb.st_size
			    && (statb.st_mtime != mailtime)
			    && mailtime
			) {	prs(mailmsg)
			FI
			mailtime=statb.st_mtime;
			prs(ps1nod.namval); alarm(TIMEOUT); flags |= waiting;
		FI

		trapnote=0; peekc=readc();
		IF eof
		) {	return;
		FI
		alarm(0); flags &= ~waiting;
		execute(cmd(NL,MTFLG),0);
		eof |= (flags&oneflg);
	}
}

chkpr(eor)
char eor;
{
	IF (flags&prompt) && standin->fstak==0 && eor==NL
	) {	prs(ps2nod.namval);
	FI
}

settmp()
{
	itos(getpid()); serial=0;
	tmpnam=movstr(numbuf,&tmpout[TMPNAM]);
}

Ldup(fa, fb)
	REG INT		fa, fb;
{
	dup2(fa, fb);
	close(fa);
        fcntl(fb, F_SETFD, FD_CLOEXEC);
}
