/*
 * Tree search generalized from Knuth (6.2.2) Algorithm T just like
 * the AT&T man page says.
 *
 * The node_t structure is for internal use only, lint doesn't grok it.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 *
 * From unix-pc.sources, August 1989
 */

#include <search.h>

typedef struct node_t
{
	char	  *key;
	struct node_t *llink, *rlink;
} node;

void *tfind(void *key, void **_rootp, __compar_fn_t compar)
/* find a node, or return 0 */
{
    node **rootp = _rootp;
    if (rootp == NULL)
	return NULL;
    while (*rootp != NULL)	/* T1: */
    {
	int r;
	if ((r = (*compar)(key, (*rootp)->key)) == 0)	/* T2: */
	    return (*rootp);		/* key found */
	rootp = (r < 0) ?
	    &(*rootp)->llink :		/* T3: follow left branch */
	    &(*rootp)->rlink;		/* T4: follow right branch */
    }
    return NULL;
}

