/*
* operator.c - operators c, d and y
*
*
* Entry points:
*
*	do_insert()
*		Read characters from the keyboard and place them in the buffer
*		at the cursor location.  Characters at and right of the cursor
*		are shifted right to make room for the new characters.  Input is
*		ended by <esc>.
*
*	operator(op, line1, pos1)
*	char op;
*	int line1, pos1;
*		Apply the operator op.  Let the location addressed in the
*		operator command be (line2, pos2), where, pos2 < 0 for a line
*		address.  With a line address, lines from line1 to line2,
*		inclusive, are affected.  With a character address, the affected
*		buffer segment extends from the earlier (in the buffer) location
*		to one position before later location.
*
*	The commands implemented by operator() are:
*
*	c<addr>	- change a buffer segment
*	d<addr>	- delete a buffer segment
*	y<addr>	- copy a buffer segment to the yank buffer
*
*
*  External procedure calls:
*
*	b_delete(first, last)				.. file Bman.c
*	int first, last;
*		Delete a range of buffer lines.
*
*	b_getcur(line_ptr, pos_ptr)			.. file Bman.c
*	int *line_ptr, *pos_ptr;
*		Return the location of the cursor.
*
*	b_gets(k, s)					.. file Bman.c
*	int k;
*	char s[];
*		Copy the k-th buffer line to s.
*
*	int b_insert(k, s)				.. file Bman.c
*	int k;
*	char s[];
*		Insert s into the buffer as line k; tell if successful.
*
*	b_replace(k, s) 				.. file Bman.c
*	int k;
*	char s[];
*		Replace the k-th buffer line by s.
*
*	b_setcur(line, pos)				.. file Bman.c
*	int line, pos;
*		Set the cursor location.
*
*	int b_size() 					.. file Bman.c
*		Return the number of lines in the buffer.
*
*	int do_yank(line1, line2)			.. file yank.c
*	int line1, line2;
*		Copy a range of lines to the yank buffer; tell if successful.
*
*	int k_getch()					.. file keyboard.c
*		Return the next character from the keyboard.
*
*	s_refresh()					.. file Sman.c
*		Bring the screen up to date after a buffer change.
*
*	s_savemsg(msg, count)				.. file Sman.c
*	char *msg;
*	int count;
*		Format msg and save it for the next screen refreshing.
*/

#include <string.h>

#include "s.h"

extern void b_getcur(), s_putmsg(), s_savemsg(), b_setcur(), b_delete();
extern void b_replace(), s_refresh(), b_gets(), b_setline();
extern int do_yank(), k_getch(), b_size(), b_insert();
static void do_delete(), in_chars();

/* do_insert - insert text */
void do_insert()
{
	in_chars(0, 0);
}

/* operator - apply operators */
void operator(op, line1, pos1)
char op;
int line1, pos1;
{
	int keep_going, line_addr, line2, pos2, size, swap, temp;
	char text[MAXTEXT-1];

	b_getcur(&line2, &pos2);
	line_addr = (pos2 < 0);
	swap = ((line2 < line1) || (line2 == line1 && pos2 < pos1));
	if (swap) {
		/* swap so that Location 1 precedes Location 2 */
		temp =  line1;
		line1 = line2;
		line2 = temp;
		temp = pos1;
		pos1 = pos2;
		pos2 = temp;
	}
	size = line2 - line1 + 1;	/* number of affected lines */

	if (!line_addr)
		keep_going = 1;
	else if ((keep_going = do_yank(line1, line2)) == 0 && op != 'y') {
		s_putmsg("Cannot yank lines; should operation be completed? ");
		keep_going = (k_getch() == 'y');
	}

	if (op == 'y') {
		if (!keep_going)
			s_savemsg("Cannot yank lines.", 0);
		else if (!line_addr)
			UNKNOWN;
		else if (size >= 5)
			s_savemsg("%d lines yanked", size);
	}

	if (op == 'y' || !keep_going)
		/* return the cursor to its initial location */
		if (swap)
			b_setcur(line2, pos2);
		else
			b_setcur(line1, pos1);
	else if (op == 'd') {
		if (size >= 5)
			s_savemsg("%d lines deleted", size);
		do_delete(line1, pos1, line2, pos2);
	} else {	/* op == 'c' */
		if (size >= 5)
			s_savemsg("%d lines changed", size);
		if (line_addr) {
			if (line1 < line2)
				b_delete(line1, line2-1); /* replace the lines.. */
			b_replace(line1, "");	/* ..by an empty line */
			b_setcur(line1, 0);	/* start on that line */
			s_refresh();		/* display all this */
			in_chars(0, 0);		/* accept input */
		} else {
			/* mark the last overwrite location */
			if (--pos2 >= 0)
				b_gets(line2, text);
			else {
				b_gets(--line2, text);
				pos2 = strlen(text) - 1;
			}
			text[pos2] = '$';
			b_replace(line2, text);
			b_setcur(line1, pos1);	/* start at the left */
			s_refresh();		/* display all this */
			in_chars(line2, pos2);	/* accept input */
		}
	}
}

/*
* do_delete - delete text from the buffer
*
* If pos1 < 0 or pos2 < 0, then complete lines from line1 to line2, inclusive,
* are deleted.  Otherwise, the deleted text starts at the first address and
* extends up to, but not including, the second address.  The first address must
* precede the second.
*/
static void do_delete(line1, pos1, line2, pos2)
int line1, pos1, line2, pos2;
{
	char text1[MAXTEXT-1], text2[MAXTEXT-1];

	if (pos1 < 0 || pos2 < 0) {	/* line address */
		b_delete(line1, line2);
		if (b_size() == 0) {
			s_savemsg("No lines in buffer.", 0);
			b_insert(1, "");
		}
		b_setline( min(line1, b_size()) );
	} else {
		/* glue the head of line1 to the tail of line2 */
		b_gets(line1, text1);
		b_gets(line2, text2);
		if (pos1 + strlen(text2 + pos2) > MAXTEXT-1) {
			UNKNOWN;
			s_savemsg("Line length exceeds %d.", MAXTEXT);
			return;
		}
		strcpy(text1 + pos1, text2 + pos2);
		if (line1 < line2)
			b_delete(line1, line2-1);
		b_replace(line1, text1);
		if (pos1 > 0 && text1[pos1] == '\0')
			--pos1;
		b_setcur(line1, pos1);
	}
}

/*
* in_chars - insert characters into the buffer
*
* Read characters from the keyboard and place them in the buffer at the cursor
* location.  Existing buffer characters are overwritten until the cursor passes
* the location indicated by in_chars()'s arguments.  Thereafter, characters in
* the current line are shifted right to accommodate the new ones.  If characters
* marked for overwriting remain when input is ended (with <esc>), then they are
* deleted from the buffer.
*/
static void in_chars(end_line, end_pos)
int end_line, end_pos;
{
	int c, cur_line, cur_pos, i, length, start_line, start_pos;
	char text[MAXTEXT-1];

	b_getcur(&cur_line, &cur_pos);
	start_line = cur_line;
	start_pos = cur_pos;
	b_gets(cur_line, text);
	length = strlen(text);
	while ((c = k_getch()) != ESCAPE) {
		switch (c) {
			case '\b':
				/* don't back up past beginning of a line ... */
				if ((cur_pos == 0) ||
				/* ... or where the insertion started */
				   ((cur_line == start_line) &&
				    (cur_pos == start_pos))) {
					UNKNOWN;
					continue;
				}
				--cur_pos;
				/* doom the backed-over character for removal */
				if (end_line == 0) {
					end_line = cur_line;
					end_pos = cur_pos;
				}
				break;
			case CR:
				/* break the current line into two lines */
				c = text[cur_pos];
				text[cur_pos] = '\0';
				b_replace(cur_line, text);
				text[cur_pos] = c;
				if (end_line == cur_line) {
					/* good chance to delete text */
					cur_pos = end_pos + 1;
					end_line = 0;
				}
				if (end_line > cur_line) {
					b_gets(++cur_line, text);
					length = strlen(text);
					cur_pos = 0;
					break;
				}
				/* shift text to create current line (strcpy()
				   is nonportable when its arguments overlap) */
				for (i = 0; (text[i] = text[cur_pos+i]) != '\0'; ++i)
					;
				length -= cur_pos;
				if (b_insert(++cur_line, text) == 0)	/* failed */
					break;
				if (end_line > 0)
					++end_line;
				cur_pos = 0;
				break;
			default:
				if (!isprint(c) && c != '\t') {
					/* unprintable character */
					UNKNOWN;
					continue;
				}
				/* insert the character */
				if (end_line == 0) {	/* not overwriting */
					if (length >= MAXTEXT-1) {
						UNKNOWN;
						s_savemsg("Line length exceeds %d.",
							MAXTEXT);
						break;
					}
					for (i = ++length; i > cur_pos; --i)
						text[i] = text[i-1];
				}
				if (text[cur_pos] == '\0')
					text[cur_pos+1] = '\0';
				text[cur_pos] = c;
				b_replace(cur_line, text);
				/* if this is the last doomed location */
				if (end_line == cur_line && end_pos == cur_pos)
					end_line = 0;
				++cur_pos;
				break;
		}
		b_setcur(cur_line, cur_pos);
		s_refresh();
	}
	if (end_line > 0)
		/* delete the doomed characters */
		do_delete(cur_line, cur_pos, end_line, end_pos + 1);

	/* position the cursor at the last inserted character */
	b_setcur(cur_line, max(cur_pos-1, 0));
}
