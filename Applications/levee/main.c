/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-1997 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by David L Parsons (orc@pell.chi.il.us).  My name may not be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS PROVIDED
 * AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "levee.h"
#include "extern.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if (SYS5 & !ST)	/* if system 5 compatable, it has signals */
#include <signal.h>
#endif

#if RMX
    extern alien token rq$get$task$tokens();	/* for unique files */
#endif

PROC
int stamp(char *s, char *template)
/* make a unique temporary file */
{
#if RMX
    token dummy;

    strcpy(s, ":work:");
    strcat(s, template);
    numtoa(&s[strlen(s)], rq$get$task$tokens(0,&dummy));
#else
    char *p;

#if UNIX
    strcpy(s, "/tmp/");
#endif

#if FLEXOS
    s[0] = 0;
#endif

#if MSDOS
    if (p=getenv("TMP")) {
	strcpy(s, p);
	if (s[strlen(s)-1] != '\\')
	    strcat(s, "\\");
    }
    else
	s[0] = 0;
#endif
#if ST
    if (p=getenv("_TMP")) {
	strcpy(s, p);
	if (s[strlen(s)-1] != '\\')
	    strcat(s, "\\");
    }
    else
	s[0] = 0;
#endif
    strcat(s, template);
    numtoa(&s[strlen(s)], getpid());
#endif
}

#if RMX|UNIX
PROC void
ctrlc(void)
/* ctrlc: RMX control-C handler */
{
    count = 0;	/* clear count, eh? */
}
#endif

#if RMX
PROC
void settty(void)
/* settty: set up the terminal for raw input */
{
    unsigned dummy;
    /* transparent mode? */
    dq$special(1,&fileno(stdin),&dummy);
    
    /* turn off control character assignments */
    strput("\033]T:C15=0,C18=0,C20=0,C21=0,C23=0\033\\");
}
#endif

PROC
void initialize(int count, char **args)
/* initialize: set up everything I can in levee */
{
    int i;
#if RMX
    int xmode = E_INIT, xquit;
#else
#if ST
    extern int mapslash;
#endif
#endif

#if 0
    signal(SIGINT, ctrlc);
#else
/*    signal(SIGINT, SIG_IGN); */
#endif
    initcon();

#if RMX
    exception(0);
    dq$trap$cc(ctrlc,&i);
#endif

#if ZTERM
    zconfig();
#endif /*ZTERM*/

#if ST
    screensz(&LINES, &COLS);
    dofscroll = LINES/2;
#endif

#if RMX
#if TERMCAP
    {	FILE *tcf;
	extern char termcap[];

	if (tcf=fopen(":termcap:","rb")) {
	    fgets(termcap,200,tcf);		/* get a line... */
	    termcap[strlen(termcap)-1] = 0;	/* erase \n at eof */
	    fclose(tcf);			/* close the file */
	}
    }
#endif /*TERMCAP*/
    settty();
#endif /*RMX*/

#if TERMCAP
    tc_init();
#endif

    version(); strput(".  Copyright (c) 1983-1989 by David Parsons");

    if (!CA || HO[0] == 0) {
	lineonly = TRUE;
        mvcur(0, 0);
        strput(CE);
	prints("(line mode)");
    }
    else
	lineonly = FALSE;

    /* initialize macro table */
    for (i = 0;i < MAXMACROS;i++)
	mbuffer[i].token = 0;
    core[0] = EOL;
	
    yank.size = ERR;		/* no yanks yet */
    
    undo.blockp = undo.ptr = 0;
    
    fillchar(adjcurr, sizeof(adjcurr), 0);
    fillchar(adjendp, sizeof(adjendp), 0);
    
    adjcurr[BTO_WD]	=	/* more practical to just leave dynamic */
    adjcurr[SENT_BACK]	=
    adjendp[BTO_WD]	=
    adjendp[FORWD]	=
    adjendp[MATCHEXPR]	=
    adjendp[PATT_BACK]	=
    adjendp[TO_CHAR]	=
    adjendp[UPTO_CHAR]	=
    adjendp[PAGE_BEGIN]	=
    adjendp[PAGE_MIDDLE]=
    adjendp[PAGE_END]	= TRUE;

    fillchar(contexts, sizeof(contexts), -1);

    stamp(undobuf, "$un");
    stamp(yankbuf, "$ya");
    stamp(undotmp, "$tm");
    
    mvcur(LINES-1,0);
#if ST
    mapslash = getenv("mapslash") != 0L;
#endif
#if RMX
    do_file(":lvrc:", &xmode, &xquit);
#else /*!RMX	system has a environment.. */
    {	char *p;
	extern char *execstr;	/* [exec.c] */

	if (p=getenv("LVRC")) {
	    strcpy(instring,p);
	    execstr = instring;
	    setcmd();
	}
    }
#endif

    ++args, --count;
    if (count > 0 && **args == '+') {
	char *p = *args;
	strcpy(startcmd, p[1] ? (1+p) : "$");
	++args, --count;
    }
    argc = 0;
    while (count-- > 0)
	expandargs(*args++, &argc, &argv);
    if (argc > 0) {
	strcpy(filenm, argv[0]);
	if (argc > 1)
	    toedit(argc);
	inputf(filenm,TRUE);
    }
    else
	filenm[0] = 0;
}

bool PROC
execmode(exec_type emode)
{
    bool more,			/* used [more] at end of line */
	 noquit;		/* quit flag for :q, :xit, :wq */
    exec_type mode;

    zotscreen = diddled = FALSE;
    noquit = TRUE;

    if (lineonly)
	println();

    mode=emode;
    do {
	prompt(FALSE,":");
	if (getline(instring))
	    exec(instring, &mode, &noquit);
	indirect = FALSE;
	if (mode == E_VISUAL && zotscreen && noquit) {	/*ask for more*/
	    prints(" [more]");
	    if ((ch=peekc()) == 13 || ch == ' ' || ch == ':')
		readchar();
	    more = (ch != ' ' && ch != 13);
	}
	else
	    more = (mode == E_EDIT);
	if (mode != E_VISUAL && curpos.x > 0)
	    println();
	else
	    mvcur(-1,0);
    } while (more && noquit);
    if (zotscreen)
	clrprompt();
    return noquit;
}

#if ST
long _STKSIZ = 4096;
long _BLKSIZ = 4096;
#endif

void main(int argc, char *argv[])
{
    initialize(argc, argv);

    diddled = TRUE;	/* force screen redraw when we enter editcore() */
    if (lineonly)
	while (execmode(E_EDIT))
	    prints("(no visual mode)");
    else
	while (execmode(editcore()))
            /* do nada */;

    unlink(undobuf);
    unlink(yankbuf);

#if ZTERM
    zclose();
#endif

    fixcon();

#if RMX
    strputs("\033]T:C15=3,C18=13,C20=5,C21=6,C23=4\033\\\n");
    dq$special(2,&fileno(stdin),&curr);
#else
    println();
#endif
    exit(0);
}
