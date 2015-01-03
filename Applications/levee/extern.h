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
#ifndef GLOBALS_D
#define GLOBALS_D
extern
char lastchar,		/* Last character read via peekc */
     ch;		/* Global command char */
extern
exec_type mode;			/* editor init state */
extern
int lastexec;			/* last exec mode command */

extern
int contexts['z'-'`'+1];	/* Labels */
		/* C O N S T A N T S */
extern
bool adjcurr[PARA_BACK+1],
     adjendp[PARA_BACK+1];
		/* A R G U M E N T S */
extern
char startcmd[];	/* initial command after read */
extern
char **argv;		/* Arguments */
extern
int  argc,		/* # arguments */
     pc;		/* Index into arguments */
		/* M A C R O   S T U F F */
extern
struct macrecord mbuffer[];
extern
struct tmacro mcr[];		/* A place for executing macros */
			/* S E A R C H   S T U F F */
extern
char dst[],			/* last replacement pattern */
     lastpatt[],		/* last search pattern */
     pattern[];
extern
int RE_start[9],		/* start of substitute argument */
    RE_size [9],		/* size of substitute argument */
    lastp;			/* last character matched in search */
extern
struct undostack undo;		/* To undo a command */
		/* R A N D O M   S T R I N G S */
		
extern
char instring[],		/* Latest input */
     filenm[],			/* Filename */
     altnm[],			/* Alternate filename */
     gcb[];			/* Command buffer for mutations of insert */
	
extern
char undobuf[],
     undotmp[],
     yankbuf[];
extern
int uread,			/* reading from the undo stack */
    uwrite;			/* writing to the undo stack */
			    /* B U F F E R S */
extern
char rcb[], *rcp,		/* last modification command */
     core[];			/* data space */
		    
extern
struct ybuf yank;		/* last deleted/yanked text */
/* STATIC INITIALIZATIONS: */
/* ttydef stuff */
#if ST | TERMCAP
extern int LINES, COLS;
#endif
#if TERMCAP
extern bool CA, canUPSCROLL;
extern char FkL,
	    CurRT,
	    CurLT,
	    CurDN,
	    CurUP;
#endif /*TERMCAP*/
extern char *TERMNAME,
	    *HO,
	    *UP,
	    *CE,
	    *CL,
	    *OL,
	    *BELL,
	    *CM,
	    *UpS,
	    *CURoff,
	    *CURon;

extern
char erasechar,
     eraseline;

extern
char ED_NOTICE[],		/* Editor version */
     ED_REVISION,		/* Small revisions & corrections */
     fismod[],			/* File is modified message */
     fisro[];			/* permission denied message */
     
extern
char *excmds[],
     wordset[],
     spaces[];
extern
struct variable vars[];
extern
int autowrite,
    autocopy,
    overwrite,
    beautify,
    autoindent,
    dofscroll,
    shiftwidth,
    tabsize,
    list,
    wrapscan,
    bell,
    magic;
/*extern 
char *suffix;	*/
/* For movement routines */
extern
int setstep[];
/* Where the last diddling left us */
extern
struct coord curpos;
    
    /* initialize the buffer */
extern
int curr,		/* Global cursor pos */
    lstart, lend,	/* Start & end of current line */
    count,		/* Latest count */
    xp, yp,		/* Cursor window position */
    bufmax,		/* End of file here */
    ptop, pend;		/* Top & bottom of CRT window */
extern
bool modified,		/* File has been modified */
     readonly,		/* is this file readonly? */
     needchar,		/* Peekc flag */
     deranged,		/* Up-arrow || down-arrow left xpos in Oz. */
     indirect,		/* Reading from an indirect file?? */
     redoing,		/* doing a redo? */
     xerox,		/* making a redo buffer? */
     newfile,		/* Editing a new file? */
     newline,		/* Last insert/delete included a EOL */
     lineonly,		/* Dumb terminal? */
     zotscreen,		/* do more after command in execmode */
     diddled;		/* force redraw when I enter editcore */
     
extern
int macro;    /* Index into MCR macro execution stack */
extern
char nlsearch;
/* movement, command codes */
extern
cmdtype movemap[];
#endif /*GLOBALS_D*/
#ifndef EXTERN_D
#define EXTERN_D
#define wc(ch)	(scan(65,'=',(ch),wordset)<65)

#if SYS5
#define fillchar(p,l,c)	memset((p),(c),(l))
#define index(s,c)	strchr((s),(c))
#else /*!SYS5*/
#if ST
#define fillchar(p,l,c)	blkfill((p),(c),(l))
#endif /*ST*/
#endif /*SYS5*/
		/* non int functions to be found elsewhere */
		
#if 0
extern findstates findCP();
extern exec_type editcore();
extern char line(), peekc(), readchar();
extern bool getline();
extern char *findparse(),*makepat();
extern bool putfile();
extern bool doyank(), deletion(), putback();
extern bool pushb(),pushi(),pushmem(),uputcmd(),
	    delete_to_undo(),move_to_undo();
#endif

#endif /*EXTERN_D*/
