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
	struct utk_widget *d = w->c.children;

	clip_set(&damage);

	/* Not in the drawing area */
	if (clip_outside(&w->c.w.rect))
		return;

	/* Clear and border the window as needed */
	utk_win_base(w);
	/* Draw all the widgets */
	while (d) {
		utk_render_widget(d);
		d = d->next;
	}
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

	clip_set(&damage);

	while (b) {
		/* We can work up from here instead */
		if (clip_covered(&b->c.w.rect))
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
	damage_set(&screen);
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
	damage_set(&w->c.w.rect);
	ui_redraw();
	utk_menu();
}

/* Create a window - always on top */
void ui_win_create(struct utk_window *w)
{
	ui_win_link(w);
	damage_set(&w->c.w.rect);
	ui_render(w);
	utk_menu();
}

/*
 *	Adjust the window size, position, or place it at the top. We fastpath
 *	window larger and a straight move to top
 */
void ui_win_adjust(struct utk_window *w, struct utk_rect *r)
{
	damage_set(&w->c.w.rect);
	clip_set(&w->c.w.rect);
	/* If we are top and the new size covers the old entirely we can fastpath it */
	if (clip_covered(r)) {
		/* We are covering the entire damaged area */
		/* Relink ourself as the top window */
		if (w != win_top) {
			ui_win_unlink(w);
			ui_win_link(w);
		}
		/* Draw - this will cover all the damage */
		rect_copy(&w->c.w.rect, r);
		w->c.w.height = w->c.w.rect.bottom - w->c.w.rect.top - 1;
		w->c.w.width = w->c.w.rect.right - w->c.w.rect.left - 1;
		clip_set(&damage);
		clip_union(r);
		damage_set(&clip);
		ui_render(w);
		utk_menu();
		return;
	}
	/* Slow path, either we are moving to top or we are smaller or both */
	/* This can be done far more optimally with multiple cliprects but not
	   clear it is worth the comoplexity. Easy to expand if it does */
	if (w != win_top) {
		/* menu change */
	}
	ui_win_delete(w);
	rect_copy(&w->c.w.rect, r);
	w->c.w.height = w->c.w.rect.bottom - w->c.w.rect.top - 1;
	w->c.w.width = w->c.w.rect.right - w->c.w.rect.left - 1;
	ui_win_create(w);
}

void ui_win_back(struct utk_window *w)
{
	damage_set(&w->c.w.rect);
	ui_win_unlink(w);
	ui_win_link_bottom(w);
	utk_menu();
	ui_redraw();
}

void ui_win_top(struct utk_window *w)
{
	ui_win_adjust(w, &w->c.w.rect);
	utk_menu();
}

void ui_init(void)
{
	/* Initialize the toolkit below us */
	utk_init();
	/* Just so we have a known state */
	damage_set(&screen);
	clip_set(&screen);
}

void ui_exit(void)
{
	utk_exit();
}
