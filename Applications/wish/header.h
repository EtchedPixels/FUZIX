/* Definitions of all the various names and macros that we need to
 * compile Wish.
 *
 * $Revision: 41.3 $ $Date: 2003/04/21 13:08:43 $
 */

#include "machine.h"

/* These defines turn off some of the capabilities
 * of the shell. They are mainly used for finding
 * the location of bugs.
 */
#undef NO_ALIAS 			/* No alias functions */
#undef NO_JOB				/* No job functions */
#undef NO_HISTORY			/* No history stuff */
#undef NO_TILDE 			/* No tilde stuff */
#undef NO_VAR				/* No variables */
#undef NO_COMLINED			/* No command line editor */
#ifdef NO_COMLINED
# define NO_CLEX			/* No command line extensions */
# define NO_BIND			/* No key bindings */
#endif

#if defined(__STDC__) && __STDC__ && !defined(MINIX1_5)
# define PROTO
# define CONST const
# ifndef __GNUC__
#  include <stdlib.h>
# endif
#else
# define CONST
#endif

#ifndef NULL				/* Generic null pointer. May need */
# define NULL	0			/* changing on some machines to */
#endif					/* (char *)0 or (void *)0 */

#ifndef SIGTYPE				/* Signal handlers return this */
# define SIGTYPE void
#endif

#undef EOF				/* Minix defines this in stdio.h */
#define EOF	-1

#ifndef MAXSIG
# define MAXSIG	27			/* Maximum signals known to Wish */
#endif

#define UNDEF	-1			/* Used when setting cbreak etc. */
#define EOS	'\0'			/* End of string terminator */
#define BADFD	-2
#define MAXARG   300			/* Max # args passed to processes */
#define MAXWORD  200			/* Only used in parse, try to lose */
#define MAXFNAME 200			/* Only used in parse, try to lose */
#define MAXPL    512                    /* Path length */
#define MAXLL   2048			/* Used in comlined & hist */
#define MAXWL    512			/* Used in clex & meta */
#define MAXCAN  1000			/* maximum number of candidates */


/* The following used to be enums,
 * but they get used an awful lot
 * with ints.
 */
#undef  TRUE
#undef  FALSE
#define TRUE  1
#define FALSE 0
#if (!defined(FREEBSD_4) && !defined(FREEBSD_5))
typedef int bool;		/* Must be int as we pass bools as f'n args */
#endif
typedef unsigned char uchar;	/* CLE uses unsigned chars throughout */

/* We use the following when debugging malloc/free */
#ifdef MALLOCDEBUG
# define free myFree
# define malloc myMalloc
#endif

#define fatal(mess) { fprints(2,"%s\n",mess); exit(1); }


/*
 * These structures are used to store the definitions for aliases, history,
 * the tilde lists, variables.
 */

struct val {
        char *name;             /* The name of the var/history/tilde/alias */
        char *val;              /* The value */
        int hnum;               /* History only: the history number */
        bool exported;          /* Variable only: is this exported? */
        struct val *next;
};

struct vallist {
        struct val *head;       /* Singly-linked list with head/tail */
        struct val *tail;
};

/* Clex and Meta
 *
 * The following structure is used by both clex.c and meta.c. Meta
 * uses it to hold candidates files that matched a ^D or <tab> expression.
 * Here the mode holds the mode of each file as obtained by stat().
 * Clex uses the struct to build a linked list of words that will
 * eventually be parsed by the parser. Here, the mode is a complex bitfield.
 * For normal words,the mode is used as a bitfield to indicate:
 *	- if the name has been malloc'd (mode&TRUE)
 *	- if the name has an invisible space at the end (mode&C_SPACE)
 *	- if the name is in single quotes (mode&C_QUOTE)
 *	- if the name is in backquotes (mode&C_BACKQUOTE)
 *	- if the name is in curly brackets (mode&C_CURLY)
 *	- if the name starts with a $ sign (mode&C_DOLLAR)
 * Often there will be nodes in the list with mode==0 & name==NULL, these
 * indicate words removed during meta and should be skipped by the parser.
 */

#define C_SPACE		002		/* Word has a space on the end */
#define C_DOLLAR	004		/* Word starts with a $ sign */
#define C_QUOTE		010		/* Word in single quotes */
/* #define C_DBLQUOTE	020		* Word in double quotes */
#define C_CURLY		020		/* Word in curly braces {} */
#define C_BACKQUOTE	040		/* Word in backquotes */
/* #define C_QUOTEBITS	060		* Dbl and back quote bits */

/* Some words have name==NULL & mode!=0; these are special words for the
 * parser. They are separate, non-malloc'd, non-quoted words, and the bits
 * used above can be reused. The bits must be in C_WORDMASK
 */
#define C_WORDMASK	01700		/* Bits must fall in here */

#define C_SEMI		 0100		/* The word is a semicolon */
#define C_DOUBLE	C_SEMI		/* Add this to `double' the symbol */
#define C_PIPE		 0200		/* The word is a pipe */
#define C_DBLPIPE	 0300		/* The word is a double pipe */
#define C_AMP		 0400		/* The word is an ampersand */
#define C_DBLAMP	 0500		/* The word is a double ampersand */
#define C_LT		 0600		/* The word is a less-than. */
#define C_LTLT		 0700		/* The word is two less-thans. */
#define C_FD		   03		/* File descriptor bits */
#define C_GT		01000		/* The word is a greater-than. Bits */
					/* in the mask C_FD hold the fd (1-2) */
#define C_GTGT		01100		/* The word is two greater-thans.Bits */
					/* in the mask C_FD hold the fd (0-9) */
struct candidate
	{ char *name;			/* The file's name */
	  struct candidate *next;	/* Next field in linked list */
	  int mode;			/* File's mode, or malloc'd bool */
	};

/* Execution
 *
 * The how parameter to execute() indicates how the process should be
 * executed.
 */

#define H_BCKGND        002             /* Process running in background */
#define H_FROMFILE	004		/* Process has input from file */
#define H_APPEND	010		/* Process is appending output */
 
/* Redirection
 *
 * The rdrct structure holds the new file descriptors for the process,
 * or their file names (if any). Redirect() gets 3 of these, and 
 * for (i=0 to 2) { if name not null, open(name); else if fd not 0,
 * dup2(fd,i); }
 * The how field can have the H_ bits described above.
 */

struct rdrct {
        int fd;                         /* The input file descriptor */
	int how;			/* How to open up the file */
        char *file;                     /* Input file's name */
        };

/* Command Line Editing
 *
 * Several old routines have been subsumed by one routine, Show().
 * Unfortunately, these are used in comlined, clex and hist, so the
 * defines that map the old onto the new have to be out here.
 */

#define insert(a,b,c)		(void)Show(a,b,c,0)
#define show(a,c)		(void)Show(a,0,0,2)
#define goend(a,b)		Show(a,b,0,3)
#define yankprev(line,pos)	prevword(line,&pos,2)
#define Beep			write(1,wbeep,beeplength)


/* Builtins
 *
 * The following structure holds the builtins. This is only used by
 * builtin.c and clex.c. Pity we can't leave it in builtin.c
 */

struct builptr {
        char *name;
#ifdef PROTO
        int  (*fptr)(int argc, char *argv[]);
#else
        int  (*fptr)();
#endif
	};

#include "proto.h"
