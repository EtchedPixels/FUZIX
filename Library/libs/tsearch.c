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
 * From unix-pc.sources August 1989
 */

#include <stdlib.h>
#include <search.h>

typedef struct node_t
{
    char	  *key;
    struct node_t *left, *right;
}
node;

void *tsearch(void *key, void **_rootp, __compar_fn_t compar)
/* find or insert datum into search tree */
{
node **rootp = _rootp;
    node *q;

    if (rootp == NULL)
	return NULL;
    while (*rootp != NULL)	/* Knuth's T1: */
    {
	int r;

	if ((r = (*compar)(key, (*rootp)->key)) == 0)	/* T2: */
	    return (*rootp);		/* we found it! */
	rootp = (r < 0) ?
	    &(*rootp)->left :		/* T3: follow left branch */
	    &(*rootp)->right;		/* T4: follow right branch */
    }
    q = (node *) malloc(sizeof(node));	/* T5: key not found */
    if (q != NULL)			/* make new node */
    {
	*rootp = q;			/* link new node to old */
	q->key = key;			/* initialize new node */
	q->left = q->right = (struct node_t *)0;
    }
    return (q);
}

void *tdelete(void *key, void **_rootp, __compar_fn_t compar)
/* delete node with given key */
{
node **rootp = _rootp;
    node *p;
    node *q;
    node *r;
    int cmp;

    if (rootp == NULL || (p = *rootp) == NULL)
	return NULL;
    while ((cmp = (*compar)(key, (*rootp)->key)) != 0)
    {
	p = *rootp;
	rootp = (cmp < 0) ?
	    &(*rootp)->left :		/* follow left branch */
	    &(*rootp)->right;		/* follow right branch */
	if (*rootp == NULL)
	    return NULL;		/* key not found */
    }
    r = (*rootp)->right;			/* D1: */
    if ((q = (*rootp)->left) == NULL)	/* Left (struct node_t *)0? */
	q = r;
    else if (r != NULL)		/* Right link is null? */
    {
	if (r->left == NULL) {	/* D2: Find successor */
	    r->left = q;
	    q = r;
	} else {			/* D3: Find (struct node_t *)0 link */
	    for (q = r->left; q->left != NULL; q = r->left)
		r = q;
	    r->left = q->right;
	    q->left = (*rootp)->left;
	    q->right = (*rootp)->right;
	}
    }
    free((struct node_t *) *rootp);	/* D4: Free node */
    *rootp = q;				/* link parent to new node */
    return(p);
}

static void trecurse(node *root, __action_fn_t action, int level)
/* Walk the nodes of a tree */
{
    if (root->left == (struct node_t *)0 && root->right == (struct node_t *)0)
	(*action)(root, leaf, level);
    else
    {
	(*action)(root, preorder, level);
	if (root->left != NULL)
	    trecurse(root->left, action, level + 1);
	(*action)(root, postorder, level);
	if (root->right != NULL)
	    trecurse(root->right, action, level + 1);
	(*action)(root, endorder, level);
    }
}

void twalk(void *root, __action_fn_t action)	/* Walk the nodes of a tree */
{
    if (root != NULL && action != NULL)
	trecurse(root, action, 0);
}

