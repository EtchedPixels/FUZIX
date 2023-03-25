/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

/*	topological sort
 *	input is sequence of pairs of items (blank-free strings)
 *	nonidentical pair is a directed edge in graph
 *	identical pair merely indicates presence of node
 *	output is ordered list of items consistent with
 *	the partial ordering specified by the graph
*/
#include <stdio.h>
#include <stdlib.h>

/*	the nodelist always has an empty element at the end to
 *	make it easy to grow in natural order
*/

struct nodelist {
	struct nodelist *nextnode;
	struct predlist *inedges;
	char *name;
	enum { DEAD, LIVE, ONCE, TWICE } live;
} firstnode = {
NULL, NULL, NULL, DEAD};

/*	a predecessor list tells all the immediate
 *	predecessors of a given node
*/
struct predlist {
	struct predlist *nextpred;
	struct nodelist *pred;
};

const char *empty = "";

/*	is i present on j's predecessor list?
*/
int present(struct nodelist *i, struct nodelist *j)
{
	register struct predlist *t;
	for (t = j->inedges; t != NULL; t = t->nextpred)
		if (t->pred == i)
			return (1);
	return (0);
}

/*	is there any live predecessor for i?
*/
int anypred(struct nodelist *i)
{
	register struct predlist *t;
	for (t = i->inedges; t != NULL; t = t->nextpred)
		if (t->pred->live == LIVE)
			return (1);
	return (0);
}

void note(const char *s, const char *t)
{
	fprintf(stderr, "tsort: %s%s\n", s, t);
}

void error(const char *s, const char *t)
{
	note(s, t);
	exit(1);
}

int cmp(const char *s, const char *t)
{
	while (*s == *t) {
		if (*s == 0)
			return (1);
		s++;
		t++;
	}
	return (0);
}

static void *xmalloc(size_t size)
{
	void *p = malloc(size);
	if (p == NULL) {
		fputs("Out of memory.\n", stderr);
		exit(1);
	}
	return p;
}

/*	turn a string into a node pointer
*/
struct nodelist *stringindex(const char *s)
{
	register struct nodelist *i;
	register const char *t;
	register char *n;
	for (i = &firstnode; i->nextnode != NULL; i = i->nextnode)
		if (cmp(s, i->name))
			return (i);
	for (t = s; *t; t++);
	n = xmalloc((unsigned) (t + 1 - s));
	i->nextnode = (struct nodelist *) xmalloc(sizeof(struct nodelist));
	if (i->nextnode == NULL || n == NULL)
		error("too many items", empty);
	i->name = n;
	i->live = LIVE;
	i->nextnode->nextnode = NULL;
	i->nextnode->inedges = NULL;
	i->nextnode->live = DEAD;
	while ((*n++ = *s++) != 0);
	return (i);
}



/*	given that there is a cycle, find some
 *	node in it
*/
struct nodelist *findloop(void)
{
	register struct nodelist *i, *j;
	register struct predlist *p;
	for (i = &firstnode; i->nextnode != NULL; i = i->nextnode)
		if (i->live == LIVE)
			break;
	note("cycle in reverse order", empty);
	while (i->live == LIVE) {
		i->live = ONCE;
		for (p = i->inedges;; p = p->nextpred) {
			if (p == NULL)
				error("error 1", "");
			i = p->pred;
			if (i->live != DEAD)
				break;
		}
	}
	while (i->live == ONCE) {
		i->live = TWICE;
		note(i->name, empty);
		for (p = i->inedges;; p = p->nextpred) {
			if (p == NULL)
				error("error 2", "");
			i = p->pred;
			if (i->live != DEAD)
				break;
		}
	}
	for (j = &firstnode; j->nextnode != NULL; j = j->nextnode)
		if (j->live != DEAD)
			j->live = LIVE;
	return (i);
}


/*	the first for loop reads in the graph,
 *	the second prints out the ordering
*/
int main(int argc, const char *argv[])
{
	register struct predlist *t;
	FILE *input = stdin;
	register struct nodelist *i, *j;
	int x;
	char precedes[50], follows[50];
	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (input == NULL)
			error("cannot open ", argv[1]);
	}
	for (;;) {
		x = fscanf(input, "%s%s", precedes, follows);
		if (x == EOF)
			break;
		if (x != 2)
			error("odd data", empty);
		i = stringindex(precedes);
		j = stringindex(follows);
		if (i == j || present(i, j))
			continue;
		t = (struct predlist *) xmalloc(sizeof(struct predlist));
		t->nextpred = j->inedges;
		t->pred = i;
		j->inedges = t;
	}
	for (;;) {
		x = 0;		/*anything LIVE on this sweep? */
		for (i = &firstnode; i->nextnode != NULL; i = i->nextnode) {
			if (i->live == LIVE) {
				x = 1;
				if (!anypred(i))
					break;
			}
		}
		if (x == 0)
			break;
		if (i->nextnode == NULL)
			i = findloop();
		printf("%s\n", i->name);
		i->live = DEAD;
	}
}
