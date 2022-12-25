/*
 *	Simplified redraw for small systems. Basically a draw from back clipped
 *	but with a couple of 'spot the obvious'
 */

#include <stdio.h>
#include "utk.h"

struct utk_window *win_top;
struct utk_window *win_bottom;

/* Render a window and contents */
static void ui_render(struct utk_window *w)
{
	struct utk_widget *d = w->widget;
	struct utk_rect tmp;

	/* Not in the drawing area */
	if (clip_outside(&w->rect))
		return;
	/* utk_win_base will set a clipping rectangle within the window
	   body. Save and restore the old one */
	clip_save(&tmp);

	/* Clear and border the window as needed */
	utk_win_base(w);
	/* Draw all the widgets */
	while (d) {
		utk_render_widget(w, d);
		d = d->next;
	}

	clip_set(&tmp);
}

/*
 *	A simple bottom to top redraw. We spot and optimize the common case
 *	where all of the area exposed is covered by a single window, but don't
 *	do all the interection and other work to get lists of cliprects as it
 *	costs us too much memory and time.
 */
void ui_redraw(void)
{
	struct utk_window *b = win_top;

	while (b) {
		/* We can work up from here instead */
		if (clip_covered(&b->rect))
			break;
		b = b->under;
	}
	if (b == NULL) {
		/* This also does the menu bar etc into the background */
		utk_fill_background();
		b = win_bottom;
	}
	while (b) {
		ui_render(b);
		b = b->over;
	}
}

void ui_redraw_all(void)
{
	clip.top = 0;
	clip.left = 0;
	clip.right = screen.right;
	clip.bottom = screen.bottom;
	ui_redraw();
}

static void ui_win_unlink(struct utk_window *w)
{
	if (w->under)
		w->under->over = w->over;
	else
		win_bottom = w->over;

	if (w->over)
		w->over->under = w->under;
	else
		win_top = w->under;
}

static void ui_win_link(struct utk_window *w)
{
	w->under = win_top;
	w->over = NULL;
	if (win_top)
		win_top->over = w;
	win_top = w;
	if (win_bottom == NULL)
		win_bottom = w;
}

/* For window cycling */
static void ui_win_link_bottom(struct utk_window *w)
{
	w->over = win_bottom;
	w->under = NULL;
	if (win_bottom)
		win_bottom->under = w;
	win_bottom = w;
	if (win_top == NULL)
		win_top = w;
}

/* Delete a window, re-render the lost area */
void ui_win_delete(struct utk_window *w)
{
	ui_win_unlink(w);
	clip_set(&w->rect);
	ui_redraw();
	/* TODO: menu change */
}

/* Create a window - always on top */
void ui_win_create(struct utk_window *w)
{
	ui_win_link(w);
	clip_set(&w->rect);
	ui_render(w);
	/* TODO: menu bar change */
}

/*
 *	Adjust the window size, position, or place it at the top. We fastpath
 *	window larger and a straight move to top
 */
void ui_win_adjust(struct utk_window *w, struct utk_rect *r)
{
	clip_set(&w->rect);
	/* If we are top and the new size covers the old entirely we can fastpath it */
	if (clip_covered(r)) {
		/* We are covering the entire damaged area */
		/* Relink ourself as the top window */
		if (w != win_top) {
			ui_win_unlink(w);
			ui_win_link(w);
			/* TODO: menu bar change */
		}
		/* Draw - this will cover all the damage */
		rect_copy(&w->rect, r);
		ui_render(w);
		return;
	}
	/* Slow path, either we are moving to top or we are smaller or both */
	/* This can be done far more optimally with multiple cliprects but not
	   clear it is worth the comoplexity. Easy to expand if it does */
	if (w != win_top) {
		/* menu change */
	}
	ui_win_delete(w);
	rect_copy(&w->rect, r);
	ui_win_create(w);
}

void ui_win_back(struct utk_window *w)
{
	clip_set(&w->rect);
	ui_win_unlink(w);
	ui_win_link_bottom(w);
	/* TODO menu change */
	ui_redraw();
}

void ui_win_top(struct utk_window *w)
{
	ui_win_adjust(w, &w->rect);
}

void ui_init(void)
{
	/* Initialize the toolkit below us */
	utk_init();
}
