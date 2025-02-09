/*
* yank.c - yank buffer
*
*
* Entry points:
*
*	do_put(way)
*	int way;
*		Copy the yank buffer to the main buffer.  If way = 1, the lines
*		go after the current line; otherwise, they go before it.
*
*	int do_yank(line1, line2)
*	int line1, line2;
*		Copy the block of lines from line1 to line2, inclusive, to the
*		yank buffer; tell if successful.  Line1 cannot exceed line2.
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
*	b_insert(k, s)					.. file Bman.c
*	int k;
*	char s[];
*		Insert s into the buffer as line k.
*
*	b_setline(line)					.. file Bman.c
*	int line;
*		Set the cursor to line's first nonwhite character.
*
*	s_savemsg(msg, count)				.. file Sman.c
*	char *msg;
*	int count;
*		Format msg and save it for the next screen refreshing.
*
*
*
* Implementation:
*
*	Simple linked list.  Storage is allocated dynamically with malloc()
*	(not ckalloc()) so execution can continue if storage is exhausted.
*/

#include <stdlib.h>
#include <string.h>

#include "s.h"

extern void b_getcur(), b_setline(), s_savemsg(), b_gets();
extern int b_insert();
void free_ybuf();

struct y_line {
	char *y_text;
	struct y_line *next;
};

static struct y_line *start = NULL;


/* do_put - copy the yank buffer to the file buffer */
void do_put(way)
int way;
{
	struct y_line *p;
	int cur_line, cur_pos, line, size;

	if (start == NULL) {
		UNKNOWN;
		return;
	}
	b_getcur(&cur_line, &cur_pos);
	if (way == 1)
		++cur_line;
	for (line = cur_line, p = start; p != NULL; p = p->next)
		b_insert(line++, p->y_text);

	/* move to first nonwhite character */
	b_setline(cur_line);
	if ((size = line - cur_line) >= 5)
		s_savemsg("%d lines added", size);
}

/* do_yank - copy lines from main buffer to yank buffer; tell if successful */
int do_yank(line1, line2)
int line1, line2;
{
	struct y_line *p, *q;
	char *r, text[MAXTEXT-1];

	p = NULL;

	free_ybuf();
	
	for ( ; line1 <= line2; ++line1) {
		b_gets(line1, text);
		q = (struct y_line *) malloc(sizeof(struct y_line));
		r = malloc((unsigned)strlen(text) +1);
		if (q == NULL || r == NULL) {
			free_ybuf();
			return(0);
		}
		q->y_text = strcpy(r, text);
		q->next = NULL;
		/* link the line in at the end of the list */
		if (start == NULL)
			start = q;
		else
			p->next = q;
		p = q;	/* p points to the end of the list */
	}
	return(1);
} 

/* free_ybuf - free the storage for the yank buffer */
void free_ybuf()
{
	struct y_line *p, *q;

	for (p = start; p != NULL; p = q) {
		free(p->y_text);
		q = p->next;
		free((char *)p);
	}
	start = NULL;
}
