/*
* screen.c - terminal-specific procedures
*
*
* Only procedures in Sman.c should access screen.c.  Entry points:
*
*	scr_clr()
*		Clear the remainder of the row, i.e., delete the characters
*		under, and to the right of, the cursor.
*	
*	scr_cls()
*		Clear all characters from the screen.
*	
*	scr_delc(i)
*	int i;
*		Delete i characters.  Characters that follow on the same row are
*		shifted left i positions and i blank characters are placed at
*		the right end of the row.
*		
*	scr_delr()
*		Delete the row under the cursor.  Later screen rows are shifted
*		up and a blank row is placed at the bottom of the screen.
*
*	scr_inr()
*		Insert a blank row at the cursor location.  Rows at and below
*		the current row are shifted down and the last row is lost.
*	
*	scr_instr(s)
*	char *s;
*		Insert the string s.  Characters under, and to the right of, the
*		cursor are shifted right.  Characters shifted beyond the right
*		margin of the screen are lost.  The calling procedure must not
*		allow s to contain tab or newline characters and is responsible
*		for resetting the cursor after a character is inserted at the
*		right margin of the screen.  No attempt should be made to insert
*		a character at the extreme lower right corner of the screen.
*	
*	scr_move(row, col)
*	int row, col;
*		Move the cursor to the given row and column of the screen.  The
*		upper left corner of the screen is considered row 1, column 1.
*	
*	scr_puts(s)
*	char *s;
*		Print the string s, overwriting existing characters.  The
*		calling procedure must not allow s to contain tab or newline
*		characters and is responsible for resetting the cursor after a
*		character is printed at the right margin of the screen. A
*		character may be printed at the extreme lower right corner of
*		the screen.
*	
*	scr_scrl()
*		Scroll screen rows up and place a blank row on the bottom.
*		The top screen row is lost.
*	
*	scr_shape(nrow_ptr, ncol_ptr)
*	int *nrow_ptr, *col_ptr;
*		Return the number of rows and columns on the screen.
*/

#include <string.h>

#include "s.h"

/* screen control commands for ANSI terminals */
#define AUTOWRAP	1
#define CLEAR_ROW	"K"
#define CLEAR_SCREEN	"2J"
#define DELETE_CHAR	"P"
#define DELETE_ROW	"M"
#define INSERT_BEGIN	"4h"
#define INSERT_END	"4l"
#define INSERT_ROW	"L"
#define LONG_COUNT	10
#define MOVE(row,col)	printf("\033[%d;%dH",row,col)
#define NCOL		80	/* columns per screen row */
#define NROW		24	/* rows per screen */
#define PAD_CHAR	'\0'
#define SCREEN(x)	printf("\033[%s",x)
#define SHORT_COUNT	4

extern void s_errmsg();
static void errmsg();
static void wait();

static int cur_row = 0, cur_col;	/* cursor location */
static char save = '\0';	/* character in location (NROW, NCOL-1) */

/* scr_clr - clear the current row */
void scr_clr()
{
	SCREEN(CLEAR_ROW);
	wait(LONG_COUNT);
}

/* scr_cls - clear the screen */
void scr_cls()
{
	SCREEN(CLEAR_SCREEN);
	wait(LONG_COUNT);
}

/* scr_delc - delete characters */
void scr_delc(i)
int i;
{
	while (i-- > 0) {
		SCREEN(DELETE_CHAR);
		wait(SHORT_COUNT);
	}
	if (cur_row == NROW)
		save = '\0';
}

/* scr_delr - delete the current row */
void scr_delr()
{
	SCREEN(DELETE_ROW);
}

/* scr_inr - insert a row */
void scr_inr()
{
	SCREEN(INSERT_ROW);
	save = '\0';
}

/* scr_instr - insert a string */
void scr_instr(s)
char *s;
{
	int s_len = strlen(s);

	if (cur_col + s_len > NCOL + 1) {
		errmsg("scr_instr(): line extends past column %d", NCOL);
		s_len = NCOL + 1 - cur_col;
		s[s_len] = '\0';
	}
	cur_col += s_len;
	if (cur_row == NROW && cur_col == NCOL + 1) {
		errmsg("scr_instr(): cannot insert in lower, right corner", 0);
		return;
	}
	SCREEN(INSERT_BEGIN);
	while (*s != '\0') {
		putchar(*s++);
		wait(SHORT_COUNT);
	}
	SCREEN(INSERT_END);
	if (AUTOWRAP && cur_col == NCOL + 1) {
		cur_col = 1;
		++cur_row;
	}
	fflush(stdout);
}

/* scr_move - move the cursor */
void scr_move(row, col)
int row, col;
{
	if (row < 1 || row > NROW)
		s_errmsg("scr_move(): illegal row %d", row);
	else if (col < 1 || col > NCOL)
		s_errmsg("scr_move(): illegal col %d", col);
	else if (col == 1 && cur_row > 0
	   && cur_row <= row && row <= cur_row + 5) {
		putchar('\r');	/* move to the start of the current row ... */
		for ( ; cur_row < row; ++cur_row)
			putchar('\n');	/* ... and down to the desired row */
		cur_col = col;
	} else if (col != cur_col || row != cur_row ) {
		MOVE(row, col);
		wait(LONG_COUNT);
		cur_row = row;
		cur_col = col;
	}
	fflush(stdout);
}

/* scr_puts - overwrite characters on the screen */
void scr_puts(s)
char *s;
{
	static int bad_bytes = 0;
	int s_len = strlen(s);
	char buf[NCOL+1], *t;

	for (t = s; *t != '\0'; ++t)
		if (!isprint(*t)) {
			if (++bad_bytes < 5)
				errmsg("scr_puts(): replacing byte with value %d by '#'",
				  (int)*t);
			*t = '#';
		}
	if (cur_col + s_len > NCOL + 1) {
		errmsg("scr_puts(): line extends past column %d", NCOL);
		s_len = NCOL + 1 - cur_col;
		s[s_len] = '\0';
	}
	cur_col += s_len;
	if (cur_row == NROW && cur_col == NCOL)
		save = s[s_len-1];
	else if (cur_row == NROW && cur_col == NCOL + 1 && s_len > 1)
		save = s[s_len-2];
	if (AUTOWRAP && cur_row == NROW && cur_col == NCOL + 1) {
		if (save == '\0') {
			scr_move(NROW, 1);
			scr_puts("Improbable display error; refresh this line.");
			scr_clr();
			fflush(stdout);
			return;
		}
		if (s_len > 1) {
			strcpy(buf, s);
			buf[s_len-2] = buf[s_len-1];
			buf[s_len-1] = '\0';
			fputs(buf, stdout);
		} else {
			scr_move(NROW, NCOL-1);
			putchar(*s);
		}
		cur_col = NCOL;
		scr_move(NROW, NCOL-1);
		SCREEN(INSERT_BEGIN);
		putchar(save);
		SCREEN(INSERT_END);
		cur_col = NCOL;
	} else {
		fputs(s, stdout);
		if (AUTOWRAP && cur_col == NCOL + 1) {
			cur_col = 1;
			++cur_row;
		}
	}
	fflush(stdout);
}

/* scr_scrl - scroll screen rows up */
void scr_scrl()
{
	scr_move(NROW, 1);
	putchar('\n');
	fflush(stdout);
}

/* scr_shape - return the number of rows and columns on the screen */	
void scr_shape(nrow_ptr, ncol_ptr)
int *nrow_ptr, *ncol_ptr;
{
	*nrow_ptr = NROW;
	*ncol_ptr = NCOL;
}

/* errmsg - print an error message and return cursor to current location */
static void errmsg(msg, val)
char *msg;
int val;
{
	int col = cur_col, row = cur_row;

	s_errmsg(msg, val);
	scr_move(row, col);
}

/* wait - pause, allowing the screen to catch up */
static void wait(count)
int count;
{
	while (count-- > 0)
		putchar(PAD_CHAR);
	fflush(stdout);
}
