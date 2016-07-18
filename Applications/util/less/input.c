/*
 * High level routines dealing with getting lines of input 
 * from the file being viewed.
 *
 * When we speak of "lines" here, we mean PRINTABLE lines;
 * lines processed with respect to the screen width.
 * We use the term "raw line" to refer to lines simply
 * delimited by newlines; not processed with respect to screen width.
 */

#include "less.h"

extern int do_bs;
extern int squeeze;
extern char *line;

/*
 * Get the next line.
 * A "current" position is passed and a "new" position is returned.
 * The current position is the position of the first character of
 * a line.  The new position is the position of the first character
 * of the NEXT line.  The line obtained is the line starting at curr_pos.
 */
POSITION
forw_line(curr_pos)
    POSITION curr_pos;
{
    POSITION new_pos;
    register int c;

    if (curr_pos == NULL_POSITION || ch_seek(curr_pos)) {
	return (NULL_POSITION);
    }

    c = ch_forw_get();
    if (c == CEOF) {
	return (NULL_POSITION);
    }

    prewind();
    for (;;) {
	if (c == '\n' || c == CEOF) {
	    /*
	     * End of the line.
	     */
	    new_pos = ch_tell();
	    break;
	}

	/*
	 * Append the char to the line and get the next char.
	 */
	if (pappend(c)) {
	    /*
	     * The char won't fit in the line; the line
	     * is too long to print in the screen width.
	     * End the line here.
	     */
	    new_pos = ch_tell() - 1;
	    break;
	}
	c = ch_forw_get();
    }
    (void) pappend('\0');

    if (squeeze && *line == '\0') {
	/*
	 * This line is blank.
	 * Skip down to the last contiguous blank line
	 * and pretend it is the one which we are returning.
	 */
	while ((c = ch_forw_get()) == '\n') {
		;
	}
	if (c != CEOF) {
	    (void) ch_back_get();
	}
	new_pos = ch_tell();
    }

    return (new_pos);
}

/*
 * Get the previous line.
 * A "current" position is passed and a "new" position is returned.
 * The current position is the position of the first character of
 * a line.  The new position is the position of the first character
 * of the PREVIOUS line.  The line obtained is the one starting at new_pos.
 */
POSITION
back_line(curr_pos)
    POSITION curr_pos;
{
    POSITION new_pos, begin_new_pos;
    int c;

    if (curr_pos == NULL_POSITION || curr_pos <= (POSITION)0 ||
	    ch_seek(curr_pos-1)) {
	return (NULL_POSITION);
    }

    if (squeeze) {
	/*
	 * Find out if the "current" line was blank.
	 */
	(void) ch_forw_get();	/* Skip the newline */
	c = ch_forw_get();	/* First char of "current" line */
	(void) ch_back_get();	/* Restore our position */
	(void) ch_back_get();

	if (c == '\n') {
	    /*
	     * The "current" line was blank.
	     * Skip over any preceeding blank lines,
	     * since we skipped them in forw_line().
	     */
	    while ((c = ch_back_get()) == '\n') {
		    ;
	    }
	    if (c == CEOF) {
		return (NULL_POSITION);
	    }
	    (void) ch_forw_get();
	}
    }

    /*
     * Scan backwards until we hit the beginning of the line.
     */
    for (;;) {
	c = ch_back_get();
	if (c == '\n') {
	    /*
	     * This is the newline ending the previous line.
	     * We have hit the beginning of the line.
	     */
	    new_pos = ch_tell() + 1;
	    break;
	}
	if (c == CEOF) {
	    /*
	     * We have hit the beginning of the file.
	     * This must be the first line in the file.
	     * This must, of course, be the beginning of the line.
	     */
	    new_pos = (POSITION)0;
	    break;
	}
    }

    /*
     * Now scan forwards from the beginning of this line.
     * We keep discarding "printable lines" (based on screen width)
     * until we reach the curr_pos.
     *
     * {{ This algorithm is pretty inefficient if the lines
     *    are much longer than the screen width, 
     *    but I don't know of any better way. }}
     */
    if (ch_seek(new_pos)) {
	return (NULL_POSITION);
    }
loop:
    begin_new_pos = new_pos;
    prewind();

    do {
	c = ch_forw_get();
	new_pos++;
	if (c == '\n') {
		break;
	}
	if (pappend(c)) {
	    /*
	     * Got a full printable line, but we haven't
	     * reached our curr_pos yet.  Discard the line
	     * and start a new one.
	     */
	    (void) pappend('\0');
	    (void) ch_back_get();
	    new_pos--;
	    goto loop;
	}
    } while (new_pos < curr_pos);

    (void)pappend('\0');

    return (begin_new_pos);
}
