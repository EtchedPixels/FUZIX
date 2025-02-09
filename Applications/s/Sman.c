/*
* Sman.c - screen manager
*
*
* Entry points:
*
*	s_errmsg(msg, val)
*	char *msg;
*	int val;
*		Format and print msg; wait for a key to be pressed.
*
*	s_finish()
*		Adjust the screen at the end of an edit session.
*
*	int s_firstline()
*		Return the number of the first line on the screen.
*
*	char *s_getmsg(msg)
*	char *msg;
*		Print msg on the last row; return the user's reply.
*
*	s_init()
*		Initialize the screen module.
*
*	int s_ismsg()
*		Tell whether a message is pending.
*
*	s_keyboard(bit)
*	int bit;
*		Record if input is from the keyboard.
*
*	int s_lastline()
*		Return the number of the last line on the screen.
*		
*	s_putmsg(msg)
*	char *msg;
*		Print msg on the last row.
*
*	s_refresh()
*		Bring the screen up to date after a buffer change.
*
*	s_savemsg(msg, val)
*	char *msg;
*	int val;
*		Format msg and save it for the next screen refreshing.
*
*
* External procedure calls:
*
*	b_getcur(line_ptr, pos_ptr)			.. file Bman.c
*	int *line_ptr, *pos_ptr;
*		Return the line and position of the cursor.
*
*	b_gets(k, s)					.. file Bman.c
*	int k;
*	char s[];
*		Copy the k-th buffer line to s.
*
*	int b_lineid(k)					.. file Bman.c
*	int k;
*		Return the ID of the k-th buffer line.
*
*	int b_size()					.. file Bman.c
*		Return the number of lines in the buffer.
*
*	int k_getch()					.. file keyboard.c
*		Return the next character of the current command.
*
*	int k_keyin()					.. file keyboard.c
*		Get a character from the keyboard.
*
*	scr_clr()					.. file screen.c
*		Clear the remainder of the row, i.e., delete the characters
*		under, and to the right of, the cursor.  Characters to the
*		left and in other rows remain.
*	
*	scr_cls()					.. file screen.c
*		Remove all characters from the screen.
*	
*	scr_delc(i)					.. file screen.c
*	int i;
*		Delete i characters.  All characters that follow on the same
*		row are shifted left i positions and i blank characters are
*		placed at the right end of the row.
*		
*	scr_delr()					.. file screen.c
*		Delete the row under the cursor.  Later rows on the screen are
*		shifted up, and a blank row is placed at the bottom of the
*		screen.
*	
*	scr_inr()					.. file screen.c
*		Insert a blank row at the cursor location.  Rows at and below
*		the current row are shifted down and the last row is lost.
*	
*	scr_instr(s)					.. file screen.c
*	char *s;
*		Insert the string s.  Characters under, and to the right of, the
*		cursor are shifted right.  Characters shifted beyond the right
*		margin of the screen are lost.  No assumption is made about
*		what happens if the line contains tabs or newline characters
*		or if the cursor reaches the right margin of the screen.
*	
*	scr_move(row, col)				.. file screen.c
*	int row, col;
*		Move the cursor to the given row and column of the screen.  The
*		upper left corner of the screen is considered row 1, column 1.
*	
*	scr_puts(s)					.. file screen.c
*	char *s;
*		Print the line s.  No assumption is made about what happens
*		if the line contains tabs or newline characters or if the cursor
*		reaches the right margin of the screen.
*	
*	scr_scrl()					.. file screen.c
*		Scroll screen rows up and place a blank row on the bottom.
*		The top screen row is lost.
*	
*	scr_shape(nrow_ptr, ncol_ptr)			.. file screen.c
*	int *nrow_ptr, *col_ptr;
*		Return the number of rows and columns on the screen.
*/

#include <string.h>
#include "s.h"

#define CLEAR		-1	/* "ID" for cleared row */
#define D_OR_I_COST	3	/* delete/insert cost per character */
#define MESSAGE		-2	/* "ID" for message row */
#define MAXCOLS		80	/* maximum possible screen width */
				/* size of buffers for expanded text */
#define MAXEXPAND	MAXTEXT+100
#define MAXROWS		24	/* maximum possible screen height */
				/* text at given segment and column */
#define TEXT		text[row+seg] + col - 1
#define TILDE		-3	/* "ID" for "~" row */
#define USEFUL		8	/* repaint screen if fewer rows can be reused */

extern void s_putmsg(), k_keyin(), scr_scrl(), scr_move(), scr_shape();
extern void scr_clr(), scr_puts(), b_getcur(), bgets(), scr_instr();
extern void scr_delr(), scr_inr(), b_gets(), scr_clc(), scr_cls(), scr_delc();
extern int k_getch(), b_lineid(), b_size();
static int after_line(), can_scroll(), expand(), good_first(), row_of_id();
static void bottom(), changes(), chop_arg(), chop_cpy(), delete();
static void display(), displ_text(), insert(), ins_text();
static void pos_to_seg(), repaint(), replace(), repl_text(), scroll();

static int
	first_line = 0,		/* line number of first screen row */
	id[MAXROWS+1],		/* ID of line at row i (subscript 0 unused) */
	keyboard,		/* is command coming from keyboard? */
	last_row,		/* last row displaying buffer contents */
	ncols,			/* number of columns on the screen */
	nrows;			/* number of rows on the screen */

static char
	msg_save[MAXCOLS+1],	/* message saved for next screen refreshing */
	*text[MAXROWS+1];	/* text of line at row i (subscript 0 unused) */

/* s_errmsg - format and print msg; wait for the user to read it */
void s_errmsg(msg, val)
char *msg;
int val;
{
	char message[MAXCOLS+1];

	sprintf(message, msg, val);
	s_putmsg(message);
	k_keyin();
}

/* s_finish - terminate the edit session */
void s_finish()
{
	scr_scrl();
	scr_move(nrows, 1);
}

/* s_firstline - return the number of the first line on the screen */
int s_firstline()
{
	return(first_line);
}

/* s_getmsg - write a message; return the reply */
char *s_getmsg(msg)
char *msg;
{
	static char last_text[MAXCOLS+1];
	char expanded[MAXCOLS+2], *reply, *s;

	strcpy(last_text, msg);
	if (keyboard)
		s_putmsg(last_text);
	s = reply = last_text + strlen(last_text);
	for ( ; s - last_text < MAXCOLS && (*s = k_getch()) != CR; ++s) {
		if (*s == '\b')
			if (s == reply) {
				s[1] = '\0';	/* return the '\b' */
				return(reply);
			} else
				s -= 2;
		else if (!isprint(*s) && *s != '\t') {
			UNKNOWN;
			--s;
			continue;
		}
		if (keyboard) {
			s[1] = '\0';
			if (expand(expanded, last_text, sizeof(expanded)) > 1)
				/* exceeds one row; don't display */
				return(reply);
			repl_text(nrows, MESSAGE, 1, 0, expanded);
		}
	}
	*s = '\0';	/* trim off the final CR */
	return(reply);
}

/* s_init - initialize for an edit session */
void s_init()
{
	int row;
	char *ckalloc();

	/* save constants giving terminal characteristics */
	scr_shape(&nrows, &ncols);
	if (ncols > MAXCOLS)
		s_errmsg("The screen has too many columns.", 0);
	else if (nrows > MAXROWS)
		s_errmsg("The screen has too many rows.", 0);
	else	/* allocate storage for remembering screen contents */
		for (row = 1; row <= nrows; ++row) {
			text[row] = ckalloc((unsigned)(ncols+1));
			strcpy(text[row], "");
		}
}

/* s_ismsg - tell if a message is waiting to be displayed */
int s_ismsg()
{
	return(msg_save[0] != '\0');
}

/* s_keyboard - record if command is from the keyboard */
void s_keyboard(bit)
int bit;
{
	keyboard = bit;
}

/* s_lastline - return the number of the last line on the screen */
int s_lastline()
{
	int last_line = first_line, row;

	for (row = 2; row <= nrows; ++row)
		if (id[row] > 0 && id[row] != id[row-1])
			++last_line;
	return(last_line);
}

/* s_putmsg - print a message on the last screen row */
void s_putmsg(msg)
char *msg;
{
	scr_move(nrows, 1);
	if (id[nrows] != CLEAR)
		scr_clr();
	id[nrows] = MESSAGE;
	expand(text[nrows], msg, ncols+1);
	scr_puts(text[nrows]);
}

/* s_refresh - refresh the screen */
void s_refresh()
{
	if (keyboard) {
		last_row = nrows - s_ismsg();
		if (first_line == 0)	/* initial refreshing */
			repaint();
		else
			changes();	/* economical refreshing */
	}
}

/* s_savemsg - save msg for the next screen refreshing */
void s_savemsg(msg, val)
char *msg;
int val;
{
	sprintf(msg_save, msg, val);
}

/* ---------  static procedures for refreshing the screen ---------- */

/* after_line - return the first screen row of the next buffer line */
static int after_line(row)
int row;
{
	while (row < nrows && id[row+1] == id[row])
		++row;
	return(row+1);
}

/* bottom - make current location visible; handle TILDE lines and messages */
static void bottom()
{
	int cur_col, cur_id, cur_line, cur_pos, cur_row, cur_seg, idr, junk,
		last_seg, n, r, tilde_row;
	char cur_text[MAXTEXT-1];

	/* guarantee that current line is completely visible */
	b_getcur(&cur_line, &cur_pos);
	b_gets(cur_line, cur_text);
	pos_to_seg(cur_text, strlen(cur_text)-1, &last_seg, &junk);
	cur_id = b_lineid(cur_line);
	n = 0;
	while ((cur_row = row_of_id(cur_id, 1)) == 0
	    || cur_row + last_seg > last_row)
		if (++n < 20)
			scroll(1, s_lastline());
		else {
			s_savemsg("Screen repainted because of display error.", 0);
			repaint();
			return;
		}

	/* fill in TILDE rows below last line of buffer */
	/* (first condition avoids long buffer search) */
	if (b_size() < first_line + nrows &&
	   (r = row_of_id(b_lineid(b_size()), cur_row)) > 0)
		for (tilde_row = after_line(r); tilde_row <= last_row; ++tilde_row)
			if ((idr = id[tilde_row]) != TILDE) {
				scr_move(tilde_row, 1);
				if (idr != CLEAR)
					scr_clr();
				scr_puts("~");
				id[tilde_row] = TILDE;
				strcpy(text[tilde_row], "~");
			}

	/* if a message is waiting, print it */
	if (s_ismsg()) {
		s_putmsg(msg_save);
		strcpy(msg_save, "");
	}

	/* move the cursor into position */
	pos_to_seg(cur_text, cur_pos, &cur_seg, &cur_col);
	if (cur_row + cur_seg <= nrows)
		scr_move(cur_row + cur_seg, cur_col);
}

/* can_scroll - try to scroll the window down; tell if successful */
static int can_scroll(new_row1, new_first)
int	new_row1,	/* row to be moved to the top of the screen */
	new_first;	/* number of the line currently at new_row1 */
{
	int count, line, row;

	/* don't scroll if the bottom part of the screen requires updating */
	for (row = new_row1, line = new_first; ++row <= nrows && id[row] > 0; )
		if (id[row] != id[row-1])
			if (++line > b_size() || id[row] != b_lineid(line))
				return(0);

	/* count lines to be removed from the top of the screen */
	for (count = row = 1; row < new_row1 - 1; ++row)
		if (id[row+1] != id[row])
			++count;

	scroll(count, line);
	return(1);
}

/* changes - economically update the screen */
static void changes()
{
	int	line,		/* buffer line being displayed */
		row,		/* row where line will begin */
		visible,	/* next buffer line already on the screen */
		useful_row,	/* row where the visible line begins */
	cur_line, cur_pos, n, new_first, last_line, partial;

	b_getcur(&cur_line, &cur_pos);

	/* determine the first buffer line that will be on the screen */
	if (cur_line >= s_firstline() && cur_line <= s_lastline() + 1)
		/* the old first line will probably display the current line */
		new_first = first_line;
	/* else compute the first line that optimally reuses existing rows */
	else if ((new_first = good_first()) == 0) {
		/* there is no good choice; repaint the screen */
		repaint();
		return;
	}

	/* determine the last displayed line, assuming one row per line */
	last_line = min(new_first + nrows - 1, b_size());

	/* record ID of line that may have segments falling below the screen */
	if ((partial = id[nrows]) <= 0)
		partial = id[nrows-1];

	for (row = 1, line = new_first;
	   row <= nrows && (row <= last_row || line == cur_line) && line <= last_line;
	   row = after_line(row), ++line) {

		/* determine the next buffer line that is already visible */
		for (visible = line; visible <= last_line; ++visible)
			if ((useful_row = row_of_id(b_lineid(visible), row)) > 0)
				break;

		if (row < useful_row && line == visible) {
			/* if screen update can be performed by scrolling .. */
			if (row == 1 && can_scroll(useful_row, new_first))
				break;
			delete(row, useful_row - 1);
		} else if (row < useful_row
		   || useful_row == 0	/* no more useful rows */
		   || id[row] == partial)	/* may need additional segments */
			replace(row, line, useful_row);
		else if (line < visible)
			/* insert in reverse order so scrolling up looks OK */
			for (n = visible - 1; n >= line; --n)
				insert(row, n);
		/* else line is already displayed at row */
	}

	first_line = new_first;
	bottom();	/* handle TILDE rows, message, etc */
}

/* chop_arg - chop a function's argument to a maximum length */
static void chop_arg(fcn, arg, maxlen)
void (*fcn)();
int maxlen;
char *arg;
{
	char save;

	save = arg[maxlen];
	arg[maxlen] = '\0';
	(*fcn)(arg);
	arg[maxlen] = save;
}

/* chop_cpy - copy at most maxlen characters from s to t; add '\0' */
static void chop_cpy(s, t, maxlen)
char *s, *t;
int maxlen;
{
	while (maxlen-- > 0 && (*s++ = *t++) != '\0')
		;
	*s = '\0';
}

/* delete - delete rows from the screen */
static void delete(from, to)
int from, to;
{
	int k, nbr_rows = to - from + 1;

	/* don't let message move up */
	if (id[nrows] == MESSAGE) {
		id[nrows] = CLEAR;
		strcpy(text[nrows], "");
		scr_move(nrows, 1);
		scr_delr();
	}

	/* remember the rows that are shifted up */
	for (k = from; k <= nrows - nbr_rows; ++k) {
		id[k] = id[k+nbr_rows];
		strcpy(text[k], text[k+nbr_rows]);
	}
	/* remember the bottom rows that are cleared */
	for (k = nrows - nbr_rows + 1; k <= nrows; ++k) {
		id[k] = CLEAR;
		strcpy(text[k], "");
	}
	/* delete rows from the screen */
	scr_move(from, 1);
	while (nbr_rows-- > 0)
		scr_delr();
}

/* display - display a line */
static void display(row, line)
int row, line;
{
	int nsegs;
	char buf[MAXTEXT-1], expanded[MAXEXPAND];

	b_gets(line, buf);
	nsegs = expand(expanded, buf, sizeof(expanded));
	displ_text(row, b_lineid(line), nsegs, expanded);
}

/* displ_text - print the text of a line */
static void displ_text(row, line_id, nsegs, s)
int row, line_id, nsegs;
char *s;
{
	int do_clear;

	for ( ; nsegs-- > 0 && row <= last_row; ++row, s += ncols) {
		scr_move(row, 1);
		do_clear = (id[row] != CLEAR && strlen(text[row]) > strlen(s));
		id[row] = line_id;
		chop_cpy(text[row], s, ncols);
		scr_puts(text[row]);
		if (do_clear)
			scr_clr();
	}
}

/* expand - expand t to s; return the number of segments */
static int expand(s, t, maxchars)
char *s, *t;
int maxchars;
{
	char *start = s;

	for ( ; s - start < maxchars - 1 && *t != '\0' ; ++s, ++t)
		if ((*s = *t) == '\t') {
			*s = ' ';	/* overwrite the tab */
			while (s - start < maxchars - 2
			    && (s - start + 1)%TAB_WIDTH != 0)
				*++s = ' ';
		}
	*s = '\0';
	return(1 + (s - start - 1)/ncols);
}

/* good_first - return a good first line for window; 0 = no good choice */
static int good_first()
{
	int best_first, cur_line, cur_pos, first, last, line, max_overlap, overlap;

	/*
	* find the window containing the current line and as many of the
	* currently visible lines as possible
	*/

	b_getcur(&cur_line, &cur_pos);

	/* start with the highest window that (probably) contains cur_line */
	first = max(cur_line - nrows + 1 + s_ismsg(), 1);

	/* determine the last possible line in the highest window */
	last = min(first + nrows - 1 - s_ismsg(), b_size());

	/* compute overlap between current screen and highest window */
	overlap = 0;
	for (line = first ; line <= last; ++line)
		if (row_of_id(b_lineid(line), 1) > 0)
			++overlap;

	/* try other possible windows */
	max_overlap = overlap;
	best_first = first;
	while (++first <= cur_line) {  /* next window */
		if (row_of_id(b_lineid(first-1), 1) > 0)
			--overlap;
		if (++last <= b_size() && row_of_id(b_lineid(last), 1) > 0)
			++overlap;
		/* in case of a tie, pick the lower window */
		if (overlap >= max_overlap) {
			max_overlap = overlap;
			best_first = first;
		}
	}
	return((max_overlap >= USEFUL) ? best_first : 0);
}

/* insert - insert a line */
static void insert(row, line)
int row, line;
{
	int nsegs;
	char buf[MAXTEXT-1], expanded[MAXEXPAND];

	b_gets(line, buf);
	nsegs = expand(expanded, buf, sizeof(expanded));
	ins_text(row, b_lineid(line), nsegs, expanded);
}

/* ins_text - insert the text of a line */
static void ins_text(row, lineid, nsegs, t)
int row, lineid, nsegs;
char *t;
{
	int r;

	nsegs = min(nsegs, nrows - row + 1);
	/* remember the rows that are shifted down */
	for (r = nrows; r >= row + nsegs; --r) {
		id[r] = id[r-nsegs];
		strcpy(text[r], text[r-nsegs]);
	}

	/* insert blank rows on the screen */
	scr_move(row, 1);
	for (r = 1; r <= nsegs; ++r)
		scr_inr();
	displ_text(row, lineid, nsegs, t);
}

/* pos_to_seg - convert a line position to a screen segment and column */
static void pos_to_seg(t, pos, seg_ptr, col_ptr)
char *t;
int pos, *seg_ptr, *col_ptr;
{
	int c, p;

	for (c = 1, p = 0; p < pos && t[p] != '\0'; ++c, ++p)
		/* keep column c corresponding to position p */
		if (t[p] == '\t')
			while (c%TAB_WIDTH != 0)
				++c;

	*seg_ptr = (c - 1)/ncols;
	*col_ptr = 1 + (c-1) % ncols;
}

/* repaint - completely repaint the screen */
static void repaint()
{
	int cur_line, cur_pos, line, row;

	/* clear the screen */
	scr_cls();
	for (row = 1; row <= nrows; ++row) {
		id[row] = CLEAR;
		strcpy(text[row], "");
	}

	b_getcur(&cur_line, &cur_pos);
	for (row = 1, line = first_line = max (cur_line - 8, 1);
	    row <= last_row && line <= b_size();
	    row = after_line(row), ++line)
		display(row, line);
	bottom();
}

/* replace - replace a line */
static void replace(row, line, useful_row)
int row, line, useful_row;
{
	int nsegs;
	char buf[MAXTEXT-1], expanded[MAXEXPAND];

	b_gets(line, buf);
	nsegs = expand(expanded, buf, sizeof(expanded));
	repl_text(row, b_lineid(line), nsegs, useful_row, expanded);
}

/* repl_text - economically replace the text of a line */
static void repl_text(row, line_id, new_segs, useful_row, new_text)
int	row,		/* row containing 0-th segment of the line */
	line_id,	/* ID of the new line */
	new_segs,	/* number of segments in the new line */
	useful_row;	/* next row that should not be overwritten */
char	*new_text;	/* text of the new line */
{
	int	add_segs,	/* number of segments to be added */
		col,		/* column of current interest */
		count,		/* character count for current segment */
		d_count,	/* number of characters to be deleted */
		do_clear,	/* is overwrite/clear strategy used? */
		i,		/* generic loop index */
		i_count,	/* number of characters to be inserted */
		o_count,	/* number of characters to be overwritten */
		old_segs,	/* number of segments in the old line */
		repl_segs,	/* number of segments to be replaced */
		seg,		/* segment of current interest */
		tail_len;	/* length of new text after mismatch */
	char	*p1,		/* first mismatching character in old text */
		*p2,		/* first mismatching character in new text */
		*s1,		/* start of matching suffix in old text */
		*s2,		/* start of matching suffix in new text */
		*t,		/* generic character pointer */
		old_text[MAXEXPAND];	/* current displayed line */

	/* build the old line from screen segments */
	strcpy(old_text, text[row]);
	for (i = row + 1; i <= nrows && id[i] == id[row] && id[i] > 0; ++i)
		strcat(old_text, text[i]);
	old_segs = i - row;
	/* don't consider segments below the screen */
	new_segs = min(new_segs, nrows - row + 1);
	/* don't replace segments that should be inserted or deleted */
	repl_segs = min(old_segs, new_segs);

	/* update id[] and text[] */
	for (seg = 0, t = new_text; seg < repl_segs; ++seg, t += ncols) {
		chop_cpy(text[row+seg], t, ncols);
		id[row+seg] = line_id;
	}

	/* point p1 and p2 to first differing characters */
	for (p1 = old_text, p2 = new_text;
		*p1 != '\0' && *p1 == *p2; ++p1, ++p2)
			;
	if (*p1 == '\0' && *p2 == '\0')		/* identical lines */
		return;

	/* point s1 and s2 to the starts of longest common suffix */
	tail_len = strlen(p2);	/* length of remainder of new line */
	for (s1 = p1 + strlen(p1), s2 = p2 + tail_len;
		s1 > p1 && s2 > p2 && s1[-1] == s2[-1]; --s1, --s2)
			;

	/* compare overwrite-clear cost against overwrite-(delete/insert) */
	d_count = s1 - p1 - (s2 - p2);	/* counts deleted chars (<0 for insert) */
	o_count = min(s1 - p1, s2 - p2);	/* counts overwritten chars */
	do_clear = (tail_len < o_count + D_OR_I_COST*abs(d_count));
	if (do_clear)
		o_count = tail_len;	/* overwrite with entire tail */

	/* move cursor to first improper character */
	seg = (p2 - new_text)/ncols;
	col = 1 + (p2 - new_text) % ncols;
	if (seg < repl_segs)
		scr_move(row + seg, col);

	/* overwrite if appropriate */
	for ( ; o_count > 0 && seg < repl_segs; o_count -= count)
		/* if overwrite operation reaches the segment's end ... */
		if ((count = ncols - col + 1) <= o_count) {
			/* overwrite with remainder of desired row */
			scr_puts(TEXT);
			/* move to start of next segment */
			if (++seg < repl_segs)
				scr_move(row + seg, col = 1);
		} else {
			/* overwrite internal substring */
			chop_arg(scr_puts, TEXT, count = o_count);
			col += count;
		}

	/* clear remainder of row if appropriate */
	if (do_clear) {
		if (d_count > 0 && seg < repl_segs)
			/* old text is longer than new text */
			scr_clr();
	/* else delete text if appropriate */
	} else if (d_count > 0)
		while (seg < repl_segs) {
			/* don't delete past the segment's end */
			count = min(d_count, ncols - col + 1);
			scr_delc(count);
			/* if there are later segments in old text ... */
			if (seg < old_segs - 1) {
				/* append characters from the next segment */
				scr_move(row + seg, col = ncols - count + 1);
				scr_puts(TEXT);
			}
			if (++seg < repl_segs)
				scr_move(row + seg, col = 1);
		}
	/* else insert text if appropriate */
	else if ((i_count = -d_count) > 0)
		while (seg < repl_segs) {
			/* if inserted text reaches the segment's end ... */
			if (i_count > ncols - col)
				scr_puts(TEXT);	/* just overwrite with it */
			else
				chop_arg(scr_instr, TEXT, i_count);
			if (++seg < repl_segs)
				scr_move(row + seg, col = 1);
		}
	/* else d_count = 0; do nothing */

	/* overwrite or insert any additional segments */
	if ((add_segs = new_segs - old_segs) > 0) {
		t = new_text + old_segs*ncols;	/* points to remaining text */
		if (useful_row == 0 || row + new_segs <= useful_row)
			displ_text(row+old_segs, line_id, add_segs, t);
		else
			ins_text(row+old_segs, line_id, add_segs, t);
	}
}

/* row_of_id - return the screen row having a given id */
static int row_of_id(i, row)
int	i,	/* ID being sought */
	row;	/* first row to be searched */
{
	for ( ; row <= nrows; ++row)
		if (id[row] == i)
			return(row);
	return(0);
}

/* scroll - scroll the window down */
static void scroll(k, line)
int	k,	/* number of lines to be pushed off the top of the screen */
	line;	/* last visible line */
{
	int	desired,	/* desired top line */
		i,		/* generic loop index */
		nsegs,		/* number of segments in last visible line */
		row,		/* row holding last visible segment */
		seg;		/* number of last visible segment */
	char	*s,		/* points to last visible segment */
		buf[MAXTEXT-1], expanded[MAXEXPAND];
	
	/* determine desired first line; initialize nsegs, row, seg and s */
	desired = first_line + k;
	b_gets(line, buf);
	nsegs = expand(expanded, buf, sizeof(expanded));
	for (row = nrows; row > 1 && id[row] < 0; --row)
		;
	for (i = row; i > 1 && id[i-1] == id[row]; --i)
		;
	seg = row - i;
	s = expanded + seg*ncols;

	/* keep adding segments or TILDE rows to produce desired first line */
	while (first_line != desired) {
		/* get next values of nsegs, seg and s */
		if (line <= b_size() && ++seg < nsegs)
			s += ncols;
		else if (++line <= b_size()) {
			b_gets(line, buf);
			nsegs = expand(expanded, buf, sizeof(expanded));
			seg = 0;
			s = expanded;
		}
		/* get next value of row; make a space for the next segment */
		if (row < nrows)
			++row;
		else {
			scr_scrl();
			if (id[1] != id[2] && id[2] > 0)
				++first_line;
			for (i = 0; i < nrows; ++i) {
				id[i] = id[i+1];
				text[i] = text[i+1];
			}
			id[nrows] = CLEAR;
			text[nrows] = text[0];
			strcpy(text[nrows], "");
			/* don't write segment then cover by a message */
			if (first_line == desired && s_ismsg())
				break;
		}
		/* write the segment or "~" */
		if (line <= b_size()) {
			scr_move(row, 1);
			if (id[row] != CLEAR)
				scr_clr();
			id[row] = b_lineid(line);
			chop_cpy(text[row], s, ncols);
			scr_puts(text[row]);
		} else if (id[row] != TILDE) {
			scr_move(row, 1);
			if (id[row] != CLEAR)
				scr_clr();
			id[row] = TILDE;
			strcpy(text[row], "~");
			scr_puts("~");
		}
	}
}
