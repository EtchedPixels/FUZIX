/*
* Bman.c - buffer manager
*
*
* Entry points:
*
*	int b_changed()
*		Tell if the buffer has been modified since the last call to
*		b_newcmd().
*
*	b_delete(from, to)
*	int from, to;
*		Manage deletion of a range of buffer lines.
*
*	b_free()
*		Free the temporary buffer storage.
*
*	b_getcur(line_ptr, pos_ptr)
*	int *line_ptr, *pos_ptr;
*		Return the line and position of the cursor.
*
*	b_getmark(line_ptr, pos_ptr)
*	int *line_ptr, *pos_ptr;
*		Return the line and position of the mark.
*
*	b_gets(k, s)
*	int k;
*	char *s;
*		Manage copying of the k-th buffer line to s.
*
*	b_init()
*		Initialize the buffer module.
*
*	b_insert(k, s)
*	int k;
*	char *s;
*		Manage insertion of s as the k-th buffer line; tell if
*		successful.
*
*	int b_lineid(k)
*	int k;
*		Return the ID of the k-th buffer line.
*
*	int b_modified()
*		Tell if the buffer has been modified since the last call to
*		b_unmod().
*
*	b_newcmd(keyboard)
*	int keyboard;
*		Record the start of a new edit command.  The argument tells
*		whether the command will be read from the keyboard.
*
*	b_replace(k, s)
*	int k;
*	char *s;
*		Manage replacement of the k-th buffer line with s.
*
*	b_setcur(line, pos)
*	int line, pos;
*		Set the cursor location.
*
*	b_setline(line)
*	int line;
*		Set the cursor to line's first nonwhite character.
*
*	b_setmark()
*		Set the mark to the cursor location.
*
*	int b_size()
*		Return the number of lines currently in the buffer.
*
*	b_unmod()
*		Record that the buffer contents match the external file.
*
*	undo()
*		Undo the most recent user command that changed the buffer.
*
*
* External procedure calls:
*
*	buf_delete(from, to)				.. file buffer.c
*	int from, to;
*		Delete a range of buffer lines.
*
*	buf_free()					.. file buffer.c
*		Free the temporary buffer storage.
*
*	buf_gets(k, s)					.. file buffer.c
*	int k;
*	char *s;
*		Copy the k-th buffer line to s.
*
*	int buf_id(k)					.. file buffer.c
*	int k;
*		Return the ID of the k-the buffer line.
*
*	buf_init()					.. file buffer.c
*		Initialize the buffer.
*
*	int buf_insert(k, s)				.. file buffer.c
*	int k;
*	char *s;
*		Insert s as the k-th buffer line; tell if successful.
*
*	int buf_replace(k, s)				.. file buffer.c
*	int k;
*	char *s;
*		Replace the k-th buffer line by s; tell if successful.
*
*	s_errmsg(msg, val)				.. file Sman.c
*	char *msg;
*	int val;
*		Format and print msg; wait for a key to be pressed.
*
*	s_savemsg(msg, val)				.. file Sman.c
*	char *msg;
*	int val;
*		Format msg and save it for the next screen refreshing.
*/

#include <stdlib.h>
#include <string.h>

#include "s.h"

/* buffer operations */
#define DELETE		1
#define INSERT		2
#define REPLACE		3

extern void s_errmsg(), buf_gets(), buf_delete(), buf_free(), buf_init();
extern void s_savemsg();
extern int b_lineid(), buf_insert(), buf_id(), buf_replace();
static void add_rec(), free_recs();

static int
	b_count = 0,		/* number of lines in the buffer */
	changed,		/* did last command change the buffer? */
	cur_line, cur_pos,	/* cursor location */
	line_prev, pos_prev,	/* origin of previous user change */
	line_start, pos_start,	/* origin of this user command */
	mark_id, mark_pos,	/* ID of marked line; position of mark in line */	
	modified;		/* does buffer differ from external file? */

/* definition of a modification record */
struct mod_rec {
	int type;		/* DELETE, INSERT or REPLACE */
	int line;		/* line number in the buffer */
	char *del_text;		/* deleted text (NULL for INSERT) */
	struct mod_rec *next;	/* link to next modification record */
};

static struct mod_rec
	*curr_recs,		/* mod recs for current user command */
	*prev_recs;		/* mod recs for previous user change */

/* b_changed - tell if last command changed the buffer */
int b_changed()
{
	return(changed);
}

/* b_delete - manage deletion of buffer lines */
void b_delete(from, to)
int from, to;
{
	int count, line;
	char text[MAXTEXT-1];

	if ((count = to - from + 1) < 0)
		s_errmsg("b_delete(): cannot delete %d lines", count);
	else if (from < 1 || to > b_count)
		s_errmsg("b_delete(): improper line number %d",
			(from < 1) ? from : to );
	else {
		for (line = from; line <= to; ++line) {
			buf_gets(line, text);
			add_rec(DELETE, from, text);
		}
		buf_delete(from, to);
		b_count -= count;
	}
}

/* b_free - manage freeing of temporary buffer storage */
void b_free()
{
	buf_free();
}

/* b_getcur - get the cursor location */
void b_getcur(line_ptr, pos_ptr)
int *line_ptr, *pos_ptr;
{
	*line_ptr = cur_line;
	*pos_ptr = cur_pos;
}

/* b_getmark - get the mark's location */
void b_getmark(line_ptr, pos_ptr)
int *line_ptr, *pos_ptr;
{
	int line;

	for (line = 1; line <= b_count; ++line)
		if (b_lineid(line) == mark_id) {
			*line_ptr = line;
			*pos_ptr = mark_pos;
			return;
		}
	*line_ptr = *pos_ptr = 0;
}

/* b_gets - manage retrieval of a buffer line */
void b_gets(k, s)
int k;
char *s;
{
	if (k < 1 || k > b_count) {
		s_errmsg("b_gets(): improper line number %d", k);
		strcpy(s, "");
	} else
		buf_gets(k, s);
}

/* b_init - manage buffer initialization */
void b_init()
{
	buf_init();
}

/* b_insert - manage insertion of s as k-th buffer line; tell if successful */
int b_insert(k, s)
int k;
char *s;
{
	if (k < 1 || k > b_count + 1)
		s_errmsg("b_insert(): improper line number %d", k);
	else if (buf_insert(k, s)) {
		add_rec(INSERT, k, (char *)NULL);
		++b_count;
		return(1);
	}
	return(0);
}

/* b_lineid - return ID of buffer line k */
int b_lineid(k)
int k;
{
	if (k < 1 || k > b_count) {
		s_errmsg("b_lineid(): improper line number %d", k);
		return(0);
	}
	return(buf_id(k));
}

/* b_modified - tell if buffer differs from external file */
int b_modified()
{
	return(modified);
}

/* b_newcmd - record the start of a command */
void b_newcmd(keyboard)
int keyboard;
{
	changed = 0;		/* even if command was pushed back on input */

	if (!keyboard)
		return;

	/*
	* It is a user command.  If the last user command changed the buffer,
	* move its modification records to the prev_recs list.
	*/
	if (curr_recs != NULL) {
		free_recs(prev_recs);
		prev_recs = curr_recs;
		curr_recs = NULL;

		/* remember where the user change started */
		line_prev = line_start;
		pos_prev = pos_start;
	}

	/* remember where the current user command started */
	line_start = cur_line;
	pos_start = cur_pos;
}

/* b_replace - manage replacement of a buffer line */
void b_replace(k, s)
int k;
char *s;
{
	char text[MAXTEXT-1];

	buf_gets(k, text);
	if (k < 1 || k > b_count)
		s_errmsg("b_replace(): improper line number %d", k);
	else if (buf_replace(k, s))
		add_rec(REPLACE, k, text);
}

/* b_setcur - set buffer's record of the cursor location */
void b_setcur(line, pos)
int line, pos;
{
	if (line < 1 || line > b_count)
		s_errmsg("b_setcur(): improper line %d", line);
	else if (pos < -1)
		/* address() uses pos == -1 to signal a line address */
		s_errmsg("b_setcur(): improper position %d", pos);
	else {
		cur_line = line;
		cur_pos = pos;
	}
}

/* b_setline - set cursor to first nonwhite character of line */
void b_setline(line)
int line;
{
	int pos;
	char text[MAXTEXT-1];

	b_gets(line, text);
	for (pos = 0; isspace(text[pos]); ++pos)
		;
	if (text[pos] == '\0')
		pos = max(pos-1, 0);
	b_setcur(line, pos);
}

/* b_setmark - set buffer's mark to the cursor location */
void b_setmark()
{
	mark_id = b_lineid(cur_line);
	mark_pos = cur_pos;
}

/* b_size - return the number of lines currently in the buffer */
int b_size()
{
	return(b_count);
}

/* b_unmod - record that the buffer matches the external file */
void b_unmod()
{
	modified = 0;
}

/* undo - undo the last user command that changed the buffer */
void undo()
{
	struct mod_rec *m;

	if (curr_recs != NULL) {
		/* happens if star operation tries to redo an undo */
		s_savemsg("Improper undo operation.");
		return;
	}

	/*
	* Undo() marches down the list of modification records generated by
	* the last user change (the list starts with the most recent change).
	* A delete is undone by an insert, and vice versa.  A replace is
	* undone by another replace.
	*/

	for (m = prev_recs; m != NULL; m = m->next)
		switch (m->type) {
			case DELETE:
				b_insert(m->line, m->del_text);
				break;
			case INSERT:
				b_delete(m->line, m->line);
				break;
			case REPLACE:
				b_replace(m->line, m->del_text);
				break;
			default:
				s_errmsg("Undo(): cannot happen", 0);
				break;
			}

	/* change starting location so this undo command can be undone */
	line_start = cur_line;
	pos_start = cur_pos;

	if (b_size() > 0)
		b_setcur(line_prev, pos_prev);
	else {
		s_savemsg("No lines in buffer.", 0);
		b_insert(1, "");
		b_setcur(1, 0);
	}
}

/* add_rec - add to the list of current modification records */
static void add_rec(type, line, del_text)
int type, line;
char *del_text;
{
	struct mod_rec *new;
	static int nospace = 0;	/* are we out of memory? */
	char *p;

	changed = modified =  1;

	/* look for the possibility of collapsing modification records */
	if ((curr_recs != NULL) && (curr_recs->line == line)
	    && (type == REPLACE) && (curr_recs->type != DELETE))
		return;

	/* do nothing if space has been exhausted */
	if (nospace)
		return;

	new = (struct mod_rec *) malloc(sizeof(struct mod_rec));
	if ((new == NULL) || ((del_text != NULL) &&
	    ((p = malloc((unsigned)strlen(del_text)+1)) == NULL))) {
		nospace = 1;
		free_recs(curr_recs);
		curr_recs = NULL;
		s_errmsg("Ran out of memory!", 0);
		return;
	}
	new->type = type;
	new->line = line;
	new->del_text = (del_text != (char *)0) ? strcpy(p, del_text) : (char *)0;
	new->next = curr_recs;
	curr_recs = new;
}

/* free_recs - free storage for modification records */
static void free_recs(m)
struct mod_rec *m;
{
	struct mod_rec *a; 

	for ( ; m != NULL; m = a) {
		a = m->next;
		if (m->del_text != NULL)
			free(m->del_text);
		free((char *)m);
	}
}
