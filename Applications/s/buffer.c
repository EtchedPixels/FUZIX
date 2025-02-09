/*
* buffer.c - data structure for the buffer
*
*
* Only procedures in Bman.c should access buffer.c.  Entry points:
*
*	buf_delete(from, to)
*	int from, to;
*		Delete a range of buffer lines.
*
*	buf_free()
*		Free temporary buffer storage.
*
*	buf_gets(k, s)
*	int k;
*	char *s;
*		Copy the k-th buffer line to s.
*
*	int buf_id(k)
*	int k;
*		Return the ID of the k-th buffer line.
*
*	buf_init()
*		Initialize the buffer.
*
*	int buf_insert(k, s)
*	int k;
*	char *s;
*		Insert s as the k-th buffer line; tell if successful.
*
*	int buf_replace(k, s)
*	int k;
*	char *s;
*		Replace the k-th buffer line by s; tell if successful.
*
*
* Implementation:
*
*	Doubly-linked list with a pointer to a recently referenced line.
*/

#include <stdlib.h>
#include <string.h>

#include "s.h"

static void reference();

struct b_line {
	char *b_text;		/* text of the line */
	int b_id;		/* ID of the line */
	struct b_line *next;	/* pointer to next line */
	struct b_line *prev;	/* pointer to previous line */
};

static struct b_line
	line0,			/* points to first and last buffer lines */
	*ref_line;		/* recently referenced line */

static int
	last_id = 0,		/* last ID assigned to a buffer line */
	ref_nbr;		/* number of recently referenced line */

/* buf_delete - delete buffer lines */
void buf_delete(from, to)
int from, to;
{
	struct b_line *b;
	int count = to - from + 1;

	reference(from);
	while (count-- > 0) {
		b = ref_line->next;
		b->prev = ref_line->prev;
		b->prev->next = b;
		free(ref_line->b_text);
		free((char *)ref_line);
		ref_line = b;
	}
}

/* buf_free - free temporary buffer storage */
void buf_free()
{
/*
* This implementation does nothing.  Implementations using a temporary file
* can unlink it here.
*/
}

/* buf_gets - get a line from the buffer */
void buf_gets(k, s)
int k;
char *s;
{
	reference(k);
	strcpy(s, ref_line->b_text);
}

/* buf_id - return the ID of a line  */
int buf_id(k)
int k;
{
	reference(k);
	return(ref_line->b_id);
}

/* buf_init - initialize the buffer */
void buf_init()
{
	line0.b_text = NULL;
	line0.b_id = 0;
	ref_line = line0.next = line0.prev = &line0;
	ref_nbr = 0;
}

/* buf_insert - insert s as the k-th buffer line; tell if successful */
int buf_insert(k, s)
int k;
char *s;
{
	struct b_line *p;
	char *q;
	
	p = (struct b_line *) malloc(sizeof(struct b_line));
	q = malloc((unsigned)strlen(s)+1);
	if (p == NULL || q == NULL)
		return(0);
	reference(k-1);
	p->b_text = strcpy(q, s);
	p->b_id = ++last_id;
	/* link node p in after node ref_line */
	p->next = ref_line->next;
	p->prev = ref_line;
	ref_line->next->prev = p;
	ref_line->next = p;
	return(1);
}

/* buf_replace - replace a buffer line; tell if successful */
int buf_replace(k, s)
int k;
char *s;
{
	char *p;

	if ((p = malloc((unsigned)strlen(s)+1)) != NULL) {
		reference(k);
		free(ref_line->b_text);
		ref_line->b_text = strcpy(p, s);
		ref_line->b_id = ++last_id;
		return(1);
	}
	return(0);
}

/* reference - point ref_line to the n-th buffer line; update ref_nbr */
static void reference(n)
int n;
{
	/* search forward from a recently referenced line ... */
	for ( ; ref_nbr < n; ++ref_nbr)
		ref_line = ref_line->next;
	/* ... or backward */
	for ( ; ref_nbr > n; --ref_nbr)
		ref_line = ref_line->prev;
}
