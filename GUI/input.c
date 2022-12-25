/*
 *	Input events
 */

#include "utk.h"

struct utk_window *utk_find_window(coord_t y, coord_t x)
{
	struct utk_window *w = win_top;
	while (w) {
		if (rect_contains(&w->rect, y, x))
			break;
		w = w->under;
	}
	return w;
}
