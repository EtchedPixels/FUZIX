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
/* global declarations */

#include "levee.h"
#define GLOBALS

char lastchar,		/* Last character read via peekc */
     ch;		/* Global command char */

exec_type mode;			/* editor init state */
int lastexec = 0;		/* last exec command */

int contexts['z'-'`'+1];	/* Labels */

		/* C O N S T A N T S */

bool adjcurr[PARA_BACK+1],
     adjendp[PARA_BACK+1];

		/* A R G U M E N T S */
char startcmd[80] = "";	/* initial command after read */
char **argv;		/* Arguments */
int  argc=0,		/* # arguments */
     pc=0;		/* Index into arguments */
#if 0
struct stat thisfile;	/* status on current file, for writeout... */
#endif

		/* M A C R O   S T U F F */
struct macrecord mbuffer[MAXMACROS];
struct tmacro mcr[NMACROS];		/* A place for executing macros */

		/* S E A R C H   S T U F F */
char dst[80] = "",			/* last replacement pattern */
     lastpatt[80] = "",			/* last search pattern */
     pattern[MAXPAT] = "";		/* encoded last pattern */

int RE_start[9],			/* start of substitution arguments */
    RE_size [9],			/* size of substitution arguments */
    lastp;				/* end of last pattern */

struct undostack undo;			/* To undo a command */


		/* R A N D O M   S T R I N G S */

char instring[80],	/* Latest input */
     filenm[80] = "",	/* Filename */
     altnm[80] = "";	/* Alternate filename */
char gcb[16];		/* Command buffer for mutations of insert */

char undobuf[40];
char undotmp[40];
char yankbuf[40];

HANDLE uread,		/* reading from the undo stack */
       uwrite;		/* writing to the undo stack */

			    /* B U F F E R S */
char rcb[256];		/* last modification command */
char *rcp;		/* this points at the end of the redo */
char core[SIZE+1];	/* data space */

struct ybuf yank;	/* last deleted/yanked text */


/* STATIC INITIALIZATIONS: */

/* ttydef stuff */

#if ST | TERMCAP
int LINES, COLS;
#endif

#if ZTERM
char *TERMNAME = "zterm",
     *HO  = "\001",	/* goto top of screen */
     *UP  = "\002",	/* move up 1 line? */
     *CE  = "\003",	/* clear to end of line */
     *CL  = "\004",	/* clearscreen */
     *OL  = "\005",	/* open current line down */
     *UpS = "\006",	/* scroll up 1 line */
     *BELL= "\007",	/* ring the bell */
     *CM  = "yes",	/* cursor movement exists */
     *CURoff,
     *CURon;
#endif /*ZTERM*/

#if ANSI
#if MSDOS
char *TERMNAME = "braindamaged ansi",
#else
char *TERMNAME = "hardwired ansi",
#endif
     *HO  = "\033[H",
     *UP  = "\033[A",
     *CE  = "\033[K",
     *CL  = "\033[H\033[J",
#if MSDOS
     *OL  = NULL,
     *UpS = NULL,
#else
     *OL  = "\033[L",
     *UpS = "\033[L",
#endif
     *BELL= "\007",
     *CM  = "\033[%d;%dH",
     *CURoff,
     *CURon;
#endif /*ANSI*/

#if VT52
#if ST
char *TERMNAME = "Atari ST",
#else
#if FLEXOS
char *TERMNAME = "Flexos console",
#else
char *TERMNAME = "hardwired vt52",
#endif /*FLEXOS*/
#endif /*ST*/
     *HO  = "\033H",
     *UP  = "\033A",
     *CE  = "\033K",
     *CL  = "\033E",
     *OL  = "\033L",
     *BELL= "\007",
     *CM  = "\033Y??",
#if FLEXOS
     *UpS = NULL,	/* Reverse scrolling is painfully slow */
#else
     *UpS = "\033I",
#endif
     *CURoff= "\033f",
     *CURon = "\033e";
#endif /*VT52*/

#if TERMCAP
bool CA, canUPSCROLL;
char FkL, CurRT, CurLT, CurUP, CurDN;

char *TERMNAME,		/* will be set in termcap handling */
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
#endif /*TERMCAP*/

char erasechar = ERASE,			/* our erase character */
     eraseline = DEL;			/* and line-kill character */

char ED_NOTICE[]  = "(c)3.4",		/* Editor version */
     ED_REVISION = 'm',			/* Small revisions & corrections */
     fismod[] = "File is modified",	/* File is modified message */
     fisro[] = "File is readonly";	/* when you can't write the file */

char *excmds[] = {
	"print",	/* lines to screen */
	"quit",		/* quit editor */
	"read",		/* add file to buffer */
	"edit",		/* replace buffer with file */
	"write",	/* write out file */
	"wq",		/* write file and quit */
	"next",		/* make new arglist or traverse this one */
	"substitute",	/* pattern */
	"xit",		/* write changes and quit */
	"file",		/* show/set file name */
	"set",		/* options */
	"rm",		/* a file */
	"previous",	/* back up in arglist */
	"delete",	/* lines from buffer */
	"=",		/* tell line number */
	"yank",		/* lines from buffer */
	"put",		/* back yanked lines */
	"visual",	/* go to visual mode */
	"exec",		/* go to exec mode */
	"insert",	/* text below current line */
	"open",		/* insert text above current line */
	"change",	/* lines */
	"undo",		/* last change */
	"!",		/* shell escape */
	"map",		/* keyboard macro */
	"unmap",	/* keyboard macro */
	"source",	/* read commands from file */
	"version",	/* print version # */
	"args",		/* print argument list */
	"rewind",	/* rewind argument list */
	NULL
};

char wordset[] = "0123456789$_#ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char spaces[] = { TAB,EOL,' ',0 };

int shiftwidth = 4,
#if TERMCAP | ST
    dofscroll,
#else
    dofscroll  = LINES/2,
#endif
    tabsize    = 8;
int autoindent = YES,
    autocopy   = NO,
    autowrite  = YES,
    wrapscan   = YES,
    overwrite  = YES,
    beautify   = YES,
    list       = NO,
    magic      = YES,
    bell       = YES,
#if ST
    mapslash,
#endif
    ignorecase = NO;

struct variable vars[]={
    {"terminal",  "",	VSTR,	V_CONST,	(void*)&TERMNAME   },
    {"shiftwidth","sw",	VINT,	0,		(void*)&shiftwidth },
    {"scroll",	  "",	VINT,	0,		(void*)&dofscroll  },
    {"tabsize",   "ts",	VINT,	V_DISPLAY,	(void*)&tabsize    },
    {"autoindent","ai",	VBOOL,	0,		(void*)&autoindent },
    {"autocopy",  "ac",	VBOOL,	0,		(void*)&autocopy   },
    {"autowrite", "aw",	VBOOL,	0,		(void*)&autowrite  },
    {"wrapscan",  "ws",	VBOOL,	0,		(void*)&wrapscan   },
    {"overwrite", "ow",	VBOOL,	0,		(void*)&overwrite  },
    {"beautify",  "be",	VBOOL,	0,		(void*)&beautify   },
    {"list",	  "",	VBOOL,	V_DISPLAY,	(void*)&list       },
    {"magic",	  "",	VBOOL,	0,		(void*)&magic      },
    {"ignorecase","ic",	VBOOL,	0,		(void*)&ignorecase },
    {"bell",      "",	VBOOL,	0,		(void*)&bell       },
#if ST
    {"mapslash",  "ms", VBOOL,	0,		(void*)&mapslash   },
#endif
    {NULL}
};

/* For movement routines */
int setstep[2] = {-1,1};

/* Where the last diddling left us */
struct coord curpos={0, 0};

    /* initialize the buffer */
int  bufmax = 0,		/* End of file here */
     lstart = 0, lend = 0,	/* Start & end of current line */
     ptop   = 0, pend = 0,	/* Top & bottom of CRT window */
     curr   = 0,		/* Global cursor pos */
     xp     = 0, yp   = 0,	/* Cursor window position */
     count  = 0;		/* Latest count */

bool modified= NO,		/* File has been modified */
     readonly= NO,		/* is this file readonly? */
     needchar= YES,		/* Peekc flag */
     deranged= NO,		/* Up-arrow || down-arrow left xpos in Oz. */
     indirect= NO,		/* Reading from an indirect file?? */
     redoing = NO,		/* doing a redo? */
     xerox   = NO,		/* making a redo buffer? */
     newfile = YES,		/* Editing a new file? */
     newline = NO,		/* Last insert/delete included a EOL */
     lineonly= NO,		/* Dumb terminal? */
     zotscreen=NO,		/* ask for [more] in execmode */
     diddled = NO;		/* force new window in editcore */

int  macro = -1;    		/* Index into MCR */
char nlsearch = 0;		/* for N and n'ing... */

/* movement, command codes */

cmdtype movemap[256]={
    /*^@*/ BAD_COMMAND,
    /*^A*/ DEBUG_C,
    /*^B*/ HARDMACRO,
    /*^C*/ BAD_COMMAND,
    /*^D*/ WINDOW_UP,
    /*^E*/ HARDMACRO,
    /*^F*/ HARDMACRO,
    /*^G*/ FILE_C,
    /*^H*/ GO_LEFT,		/* also leftarrow  */
    /*^I*/ REDRAW_C,
    /*^J*/ GO_DOWN,		/* also downarrow  */
    /*^K*/ GO_UP,		/* also uparrow    */
    /*^L*/ GO_RIGHT,		/* also rightarrow */
    /*^M*/ CR_FWD,
    /*^N*/ BAD_COMMAND,
    /*^O*/ BAD_COMMAND,
    /*^P*/ BAD_COMMAND,
    /*^Q*/ BAD_COMMAND,
    /*^R*/ BAD_COMMAND,
    /*^S*/ BAD_COMMAND,
    /*^T*/ BAD_COMMAND,
    /*^U*/ WINDOW_DOWN,
    /*^V*/ BAD_COMMAND,
    /*^W*/ BAD_COMMAND,
    /*^X*/ BAD_COMMAND,
    /*^Y*/ HARDMACRO,
    /*^Z*/ BAD_COMMAND,
    /*^[*/ BAD_COMMAND,
    /*^\*/ BAD_COMMAND,
    /*^]*/ BAD_COMMAND,
    /*^^*/ BAD_COMMAND,
    /*^_*/ BAD_COMMAND,
    /*  */ GO_RIGHT,
    /*! */ BAD_COMMAND,
    /*" */ BAD_COMMAND,
    /*# */ BAD_COMMAND,
    /*$ */ TO_EOL,
    /*% */ MATCHEXPR,
    /*& */ RESUBST_C,
    /*\ */ TO_MARK_LINE,
    /*( */ SENT_BACK,
    /*) */ SENT_FWD,
    /** */ BAD_COMMAND,
    /*+ */ CR_FWD,
    /*, */ BAD_COMMAND,
    /*- */ CR_BACK,
    /*. */ REDO_C,
    /*/ */ PATT_FWD,
    /*0 */ BAD_COMMAND,
    /*1 */ BAD_COMMAND,
    /*2 */ BAD_COMMAND,
    /*3 */ BAD_COMMAND,
    /*4 */ BAD_COMMAND,
    /*5 */ BAD_COMMAND,
    /*6 */ BAD_COMMAND,
    /*7 */ BAD_COMMAND,
    /*8 */ BAD_COMMAND,
    /*9 */ BAD_COMMAND,
    /*: */ COLIN_C,
    /*; */ BAD_COMMAND,
    /*< */ ADJUST_C,
    /*= */ BAD_COMMAND,
    /*> */ ADJUST_C,
    /*? */ PATT_BACK,
    /*@ */ BAD_COMMAND,
    /*A */ A_AT_END,
    /*B */ BACK_WD,
    /*C */ HARDMACRO,
    /*D */ HARDMACRO,
    /*E */ BAD_COMMAND,
    /*F */ BACK_CHAR,
    /*G */ GLOBAL_LINE,
    /*H */ PAGE_BEGIN,
    /*I */ I_AT_NONWHITE,
    /*J */ JOIN_C,
    /*K */ BAD_COMMAND,
    /*L */ PAGE_END,
    /*M */ PAGE_MIDDLE,
    /*N */ BSEARCH,
    /*O */ OPENUP_C,
    /*P */ PUT_AFTER,
    /*Q */ EDIT_C,
    /*R */ BIG_REPL_C,
    /*S */ BAD_COMMAND,
    /*T */ BACKTO_CHAR,
    /*U */ BAD_COMMAND,
    /*V */ BAD_COMMAND,
    /*W */ FORWD,
    /*X */ HARDMACRO,
    /*Y */ HARDMACRO,
    /*Z */ ZZ_C,
    /*[ */ BAD_COMMAND,
    /*\ */ BAD_COMMAND,
    /*] */ BAD_COMMAND,
    /*^ */ NOTWHITE,
    /*_ */ BAD_COMMAND,
    /*` */ TO_MARK,
    /*a */ APPEND_C,
    /*b */ BACK_WD,
    /*c */ CHANGE_C,
    /*d */ DELETE_C,
    /*e */ FORWD,
    /*f */ TO_CHAR,
    /*g */ BAD_COMMAND,
    /*h */ GO_LEFT,
    /*i */ INSERT_C,
    /*j */ GO_DOWN,
    /*k */ GO_UP,
    /*l */ GO_RIGHT,
    /*m */ MARKER_C,
    /*n */ FSEARCH,
    /*o */ OPEN_C,
    /*p */ PUT_BEFORE,
    /*q */ BAD_COMMAND,
    /*r */ REPLACE_C,
    /*s */ HARDMACRO,
    /*t */ UPTO_CHAR,
    /*u */ UNDO_C,
    /*v */ BTO_WD,
    /*w */ TO_WD,
    /*x */ HARDMACRO,
    /*y */ YANK_C,
    /*z */ REWINDOW,
    /*{ */ PARA_BACK,
    /*| */ TO_COL,
    /*} */ PARA_FWD,
    /*~ */ TWIDDLE_C,
    /*^?*/ BAD_COMMAND,
    /*80*/ BAD_COMMAND,
    /*81*/ BAD_COMMAND,
    /*82*/ BAD_COMMAND,
    /*83*/ BAD_COMMAND,
    /*84*/ BAD_COMMAND,
    /*85*/ BAD_COMMAND,
    /*x6*/ BAD_COMMAND,
    /*x7*/ BAD_COMMAND,
    /*x8*/ BAD_COMMAND,
    /*x9*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND,
    /*xx*/ BAD_COMMAND
    };
