#ifndef LESS_H
#define LESS_H
/*
 * Standard include file for "less".
 */
#include <sys/types.h>
#include <unistd.h>

/*
 * Special types and constants.
 */
typedef long		POSITION;

/* Special char value for EOF */
#define CEOF (0)

/*
 * {{ Warning: if POSITION is changed to other than "long",
 *    you may have to change some of the printfs which use "%ld"
 *    to print a variable of type POSITION. }}
 */

#define	END_POSITION	((POSITION)(-2))
#define	NULL_POSITION	((POSITION)(-1))

/* How quiet should we be? */
#define	NOT_QUIET	0	/* Ring bell at eof and for errors */
#define	LITTLE_QUIET	1	/* Ring bell only for errors */
#define	VERY_QUIET	2	/* Never ring bell */

/* How should we prompt? */
#define	PR_SHORT	0	/* Prompt with colon */
#define	PR_MEDIUM	1	/* Prompt with message */
#define	PR_LONG		2	/* Prompt with longer message */

/* How should we handle backspaces? */
#define	BS_UNDERLINE	0	/* Underlining converted to underline mode */
#define	BS_NORMAL	1	/* \b treated as normal char; actually output */
#define	BS_CONTROL	2	/* \b treated as control char; prints as ^H */

/* Flag to eq_message() telling what to put in the message */
#define	MNAME		001	/* File name */
#define	MOF		002	/* "file x of y" */
#define	MBYTE		004	/* "byte x/y" */
#define	MPCT		010	/* Percentage into the file */

/* Special chars used to tell put_line() to do something special */
#define	UL_CHAR		'\201'	/* Enter underline mode */
#define	UE_CHAR		'\202'	/* Exit underline mode */

#define	CONTROL(c)		((c)&037)

#include "funcs.h"

#endif /* LESS_H */
