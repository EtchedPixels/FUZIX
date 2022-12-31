/*
 *	Input events
 */

#include "utk.h"

/* A special case where we don't walk the subtree */
struct utk_window *utk_find_window(coord_t y, coord_t x)
{
	struct utk_window *w = win_top;
	while (w) {
		if (rect_contains(&w->c.w.rect, y, x))
			break;
		w = w->under;
	}
	return w;
}
