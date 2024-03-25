/*
 *	Clipping rectangle manipulation
 */

#include <stdio.h>
#include "utk.h"

void rect_copy(struct utk_rect *to, struct utk_rect *from)
{
	to->top = from->top;
	to->left = from->left;
	to->right = from->right;
	to->bottom = from->bottom;
}

unsigned rect_contains(struct utk_rect *r, coord_t y, coord_t x)
{
	if (y < r->top || y > r->bottom)
		return 0;
	if (x < r->left || x > r->right)
		return 0;
	return 1;
}

struct utk_rect clip;
struct utk_rect damage;
struct utk_rect screen;

/* True if the rectangle passed covers the entire clip area */
unsigned clip_covered(struct utk_rect *r)
{
	if (r->right < clip.right)
		return 0;
	if (r->left > clip.left)
		return 0;
	if (r->top > clip.top)
		return 0;
	if (r->bottom < clip.bottom)
		return 0;
	return 1;
}

/* True if the rectangle passed is entirely outside the drawing area */
unsigned clip_outside(struct utk_rect *r)
{
	/* Off the sides ? */
	if (r->right < clip.left || r->left > clip.right)
		return 1;
	/* Entirely above or below */
	if (r->top > clip.bottom || r->bottom < clip.top)
		return 1;
	return 0;
}

/* Compute the intersection of a rectange and clip */
void clip_intersect(struct utk_rect *r)
{
	if (r->left > clip.left)
		clip.left = r->left;
	if (r->right < clip.right)
		clip.right = r->right;
	if (r->top > clip.top)
		clip.top = r->top;
	if (r->bottom < clip.bottom)
		clip.bottom = r->bottom;
}

/* Expand the clip to cover the passed region. We might keep clip lists
   one day but for now just grow it */
void clip_union(struct utk_rect *r)
{
	if (r->left < clip.left)
		clip.left = r->left;
	if (r->right > clip.right)
		clip.right = r->right;
	if (r->top < clip.top)
		clip.top = r->top;
	if (r->bottom > clip.bottom)
		clip.bottom = r->bottom;
}

void clip_save(struct utk_rect *r)
{
	r->top = clip.top;
	r->left = clip.left;
	r->right = clip.right;
	r->bottom = clip.bottom;
}

void clip_set(struct utk_rect *r)
{
	clip.top = r->top;
	clip.left = r->left;
	clip.right = r->right;
	clip.bottom = r->bottom;
}

void damage_set(struct utk_rect *r)
{
	damage.top = r->top;
	damage.left = r->left;
	damage.right = r->right;
	damage.bottom = r->bottom;
	if (damage.top < 0)
		damage.top = 0;
	if (damage.left < 0)
		damage.left = 0;
	if (damage.right > screen.right)
		damage.right = screen.right;
	if (damage.bottom > screen.bottom)
		damage.bottom = screen.bottom;
}
