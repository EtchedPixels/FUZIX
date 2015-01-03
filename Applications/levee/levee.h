/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1980-1997 David L Parsons
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
 *
 *			  Levee v3.C
 *   C version for Unix/Atari ST/MS-DOS/OS-2/FlexOs/iRMX/etc
 *            Pascal version for UCSD Pascal 4.X
 *
 *	written fall 82' - now (approx) by David L. Parsons.
 *
 *		  many patches, suggestions,
 *		and impractical design goals by:
 *			Jim Bolland,
 *			John Plocher,
 *			John Tainter
 */
#ifndef LEVEE_D

#define LEVEE_D
#define	TOUPPER_FTN	/* defined if the libraries support toupper as */
			/* a function call */

#ifndef TRUE
#define	TRUE	(1)	/* Nobody defines TRUE & FALSE, so I will do */
#define	FALSE	(0)	/* it myself */
#endif

#define	HANDLE	int	/* default file handle type */

#define PROC		/* for magic function types (MSDOS) */

/*
 * Compilation defines for different systems.
 * Choose only one from each group.
 */
/* system you are compiling Levee on */
#define ST	0
#define RMX	0
#define UNIX	1
#define MSDOS	0
#define	FLEXOS	0

/* do your libraries follow system V standards? */
#define SYS5	1

/* what sort of terminal are you emulating? */
#define VT52	1		/* this must be nonzero for the Atari ST */
#define TERMCAP	0
#define ZTERM	0
#define ANSI	0

#if ST

#include <stdio.h>

#define	void	int	/* Alcyon C don't know void */

/* extractions from osbind.h */
#define OPEN_OLD(n)		gemdos(0x3d,n,/*open mode*/0)
#define OPEN_NEW(n)		gemdos(0x3c,n,/*permissions*/0)
#define CLOSE_FILE(f)		gemdos(0x3e,f)
#define SEEK_POSITION(f,o,m)	gemdos(0x42,(long)(o),f,m)
#define READ_TEXT(f,b,c)	gemdos(0x3f,f,(long)(c),b)
#define WRITE_TEXT(f,b,c)	gemdos(0x40,f,(long)(c),b)

extern char *malloc();
extern long gemdos();

#endif /*ST*/

#if RMX
#include <:inc:stdio.h>
#include <:inc:udi.h>

#define OPEN_OLD(n)		open(n, /*open mode*/0)
#define OPEN_NEW(n)		creat(n,/*permissions*/0)
#define CLOSE_FILE(f)		close(f)
#define SEEK_POSITION(f,o,m)	lseek((f),(long)(o),(m))
#define READ_TEXT(f,p,c)	read((f),(p),(unsigned)(c))
#define WRITE_TEXT(f,p,c)	write((f),(p),(unsigned)(c))

#define zwrite(p,s)	write(1,(p), (unsigned)(s))

#endif /*RMX*/

#if MSDOS

#include <stdio.h>
#include <fcntl.h>

#define OPEN_OLD(n)		open(n, O_RDONLY|O_BINARY)
#define OPEN_NEW(n)		open(n, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666)
#define CLOSE_FILE(f)		close(f)
#define SEEK_POSITION(f,o,m)	lseek(f, (long)(o), (m))
#define READ_TEXT(f,p,c)	read(f, p, (int)(c))
#define WRITE_TEXT(f,p,c)	write(f, p, (int)(c))

#define zwrite(p,s)	WRITE_TEXT(fileno(stdout), p, s)

#undef PROC
#define PROC	_fastcall

#include "proto.h"

#endif /*MSDOS*/

#if UNIX

#include <stdio.h>
#include <fcntl.h>

#define OPEN_OLD(n)		open(n, O_RDONLY)
#define OPEN_NEW(n)		open(n, O_WRONLY|O_CREAT|O_TRUNC, 0666)
#define CLOSE_FILE(f)		close(f)
#define SEEK_POSITION(f,o,m)	lseek(f, (long)(o), (m))
#define READ_TEXT(f,p,c)	read(f, p, (int)(c))
#define WRITE_TEXT(f,p,c)	write(f, p, (int)(c))

#define zwrite(p,s)	WRITE_TEXT(fileno(stdout), p, s)

#endif /*MSDOS*/

#if FLEXOS

#undef TOUPPER_FTN			/* Nope, gotta do macro */

#include <stdio.h>

#define OPEN_OLD(n)		s_open(m,0x08)		/* note reversed parameters! */
#define OPEN_NEW(n)		s_create(0,0,n,0,0/*mode*/,0)
#define CLOSE_FILE(f)		s_close(0,f)		/* Full close on handle */
#define SEEK_POSITION(f,o,m)	s_seek((m&03)<<9, f, o)
#define	READ_TEXT(f,p,c)	s_read(0x0100,(long)(f),p,(long)(c),0L)
#define	WRITE_TEXT(f,p,c)	s_write(0x0101,(long)(f),p,(long)(c),0L)

#define	zwrite(p,s)		s_write(0x0101, 1L, p, (long)(s),0L)

#define	unlink(n)		s_delete(0, n)
#define	rename(a,b)		s_rename(0, a, b)

/* OPEN_OLD mode flags */

#undef	HANDLE
#define	HANDLE	long

#endif /*FLEXOS*/

#define bool int

/* ttydef stuff */
#if !(ST | TERMCAP)

#ifndef LINES
#define LINES	25
#endif  /*LINES*/
#define COLS	79

#endif

#define	YES	1
#define	NO	0

#define UPARROW	11
#define DNARROW	10
#define LTARROW	erase
#define RTARROW	12

#if !TERMCAP
#define CA	TRUE
#define canUPSCROLL !(MSDOS|FLEXOS)
#endif

/* nospecific stuff */
#define MAGICNUMBER  42
#define hell_freezes_over  FALSE
#define BUGS	7	/* sometime when you least expect it.. */

#define DW	23	/* Delete Word */
#define EOL	10	/* End Of Line */
#define DLE	16	/* Space compression lead-in */
#define ESC	27	/* Escape */

/* hardwired line kill and erase character for non-unix machines */
#define DEL	21	/* ^U */
#if RMX
#define ERASE	127
#else
#define ERASE	8	/* ^H */
#endif

#define TAB	9
    
	/* variable types */
#define VBOOL	0
#define VINT	1
#define VSTR	2

#define ERR	(-1)

		/* Undostack commands */
#define U_ADDC	'A'
#define U_MOVEC	'M'
#define U_DELC	'D'

		/* magic things for find */
#define MAXPAT	((int)300)

		/* exec mode commands */
#define	EX_CR		(ERR-1)
#define EX_PR		0
#define EX_QUIT		1
#define EX_READ		2
#define EX_EDIT		3
#define EX_WRITE	4
#define EX_WQ		5
#define EX_NEXT 	6
#define EX_SUBS 	7
#define EX_XIT		8
#define EX_FILE 	9
#define EX_SET		10
#define EX_RM		11
#define EX_PREV 	12
#define EX_DELETE	13
#define EX_LINE		14
#define EX_YANK		15
#define EX_PUT		16
#define EX_VI		17
#define EX_EX		18
#define EX_INSERT	19
#define EX_OPEN		20
#define EX_CHANGE	21
#define EX_UNDO		22
#define EX_ESCAPE	23
#define EX_MAP		24
#define EX_UNMAP	25
#define EX_SOURCE	26
#define EX_VERSION	27
#define EX_ARGS		28
#define	EX_REWIND	29

		/* movement return states */
#define LEGALMOVE	0
#define BADMOVE		1
#define ESCAPED		2
#define findstates char

		/* command codes */
#define BAD_COMMAND	0
		/*visual movement*/
#define GO_RIGHT	1
#define GO_LEFT		2
#define GO_UP		3
#define GO_DOWN		4
#define FORWD		5
#define TO_WD		6
#define BACK_WD		7
#define BTO_WD		8
#define NOTWHITE	9
#define TO_COL		10
#define TO_EOL		11
#define MATCHEXPR	12
#define TO_CHAR		13
#define UPTO_CHAR	14
#define BACK_CHAR	15
#define BACKTO_CHAR	16
#define SENT_FWD	17
#define SENT_BACK	18
#define PAGE_BEGIN	19
#define PAGE_END	20
#define PAGE_MIDDLE	21
#define CR_FWD		22
#define CR_BACK		23
#define PATT_FWD	24
#define PATT_BACK	25
#define FSEARCH		26
#define BSEARCH		27
#define GLOBAL_LINE	28
#define TO_MARK		29
#define TO_MARK_LINE	30
#define PARA_FWD	31
#define PARA_BACK	32
		/*modifications*/
#define DELETE_C	40
#define ADJUST_C	41
#define CHANGE_C	42
#define YANK_C		43
#define INSERT_C	44
#define APPEND_C	45
#define I_AT_NONWHITE	46
#define A_AT_END	47
#define OPEN_C		48
#define OPENUP_C	49
#define REPLACE_C	50
#define TWIDDLE_C	51
#define RESUBST_C	52
#define JOIN_C		53
#define UNDO_C		54
#define BIG_REPL_C	55
#define PUT_BEFORE	56
#define PUT_AFTER	57
		/*everything else*/
#define HARDMACRO	70
#define REWINDOW	71
#define ZZ_C		72
#define DEBUG_C		73
#define FILE_C		74
#define WINDOW_UP	75
#define WINDOW_DOWN	76
#define REDRAW_C	77
#define MARKER_C	78
#define REDO_C		79
#define EDIT_C		80
#define COLIN_C		81
		    /*macros*/
#define SOFTMACRO	100
#define INSMACRO	101
#define cmdtype		char

		/* exec mode states */
#define E_VISUAL 0
#define E_INIT	 1
#define E_EDIT	 2
#define exec_type	char

		/* various sizes */
#define INSSIZE	 ((int)80)		/* Insert string size */
#define FSIZE	 ((int)39)		/* File string size */
#ifndef SIZE
# define SIZE	 ((int)4096)		/* Edit buffer size */
#endif

#define SBUFSIZE  ((int)512)		/* Incore yank buffer size */
#define MAXMACROS 32			/* Maximum # of macros */
#define NMACROS	  9			/* Nexting level for macros */

#define PAGESIZE ((int)512)		/* Bytes per block */

struct coord {		/* Screen Coordinate */
    int x,y;
};

struct ybuf {		/* Yank Buffer */
    int size;				/* Bytes yanked */
    bool lines,				/* Yanked whole lines? */
	has_eol;			/* Yanked a EOL? */
    char stuff[SBUFSIZE];		/* The stuff */
};

struct undostack {	/* Undo Stack Descriptor */
    int blockp,				/* block address of core block */
	ptr;				/* offset within coreblock */
    int coreblock[PAGESIZE];		/* core block */
};

union optionrec {	/* Black Magic Option Structure */
    int valu;				/* if integer, the value */
    char *strp;				/* or if string, a pointer to it */
};

struct macrecord {	/* Macro Descriptor */
    char token;				/* Character mapped */
    cmdtype oldmap;			/* Old value in movemap */
    char *m_text;			/* Replacement text */
};
		    
struct tmacro {		/* For running a macro */
    char *mtext,	/* Pointer to macro text */
	 *ip;		/* Pointer into macro text */
    int  m_iter;	/* Number of times to execute */
};

#define V_CONST	1	/* this option cannot be modified */
#define V_DISPLAY 2	/* this option affects the display */

struct variable {	/* Settable Variable Record */
    char *v_name;		/* full name */
    char *v_abbr;		/* abbreviated name */
    int v_tipe;			/* what kind of variable */
    int v_flags;		/* special attributes... */
    union optionrec *u;		/* pointer to it */
};

#include "proto.h"

#endif /*LEVEE_D*/
