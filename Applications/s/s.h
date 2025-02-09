/*
* s.h - macro definitions for the screen editor
*/
#include <stdio.h>
#include <ctype.h>

#define CR		'\r'		/* sent by <return> key */
#define ctrl(x)		(x & 037)	/* control character 'x' */
#define ESCAPE		27		/* end-of-insertion character */
#define MAXTEXT		1000		/* maximum length of a line */
#define SCROLL_SIZE	12		/* number of rows to scroll */
#define TAB_WIDTH	8		/* columns per tab character */

#define abs(x)		((x > 0) ? x : -(x))
#define max(x,y)	(x > y) ? x : y
#define min(x,y)	(x < y) ? x : y

/* for an unknown command, ring the bell */
#define UNKNOWN  putc(7, stderr)
