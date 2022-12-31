#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include "utk.h"

struct utk_window *ev_win;	/* Window for multi-part event handling */
coord_t cursor_y;
coord_t cursor_x;
coord_t save_y;
coord_t save_x;

static uint8_t *vidmem;
static uint8_t *dirt_l;
static uint8_t *dirt_r;

/*
 *	Draw routine. Pretty minimal - optimize later
 */

static void refresh_slice(uint8_t * p, uint_fast8_t y)
{
	uint_fast8_t x = dirt_l[y];
//    move_to(y, x);
	printf("\033[%d;%dH", y + 1, x + 1);
	fwrite(p + x, dirt_r[y] - dirt_l[y] + 1, 1, stdout);
}

/* TODO: would be faster to make dirt an array of l/r struct */
void utk_refresh(void)
{
	uint_fast8_t y = 0;
	uint8_t *p = vidmem;

	while (y <= screen.bottom) {
		if (dirt_l[y] != 255)
			refresh_slice(p, y);
		dirt_l[y] = 255;
		dirt_r[y] = 0;
		p += screen.right + 1;
		y++;
	}
	printf("\033[%d;%dH", cursor_y + 1, cursor_x + 1);
	fflush(stdout);
}

/*
 *	These routines don't deal with clipping and assume the request
 *	is already clipped to line in the display and in the rneder area
 *
 *	All output actually ends up on the render buffer and is written
 *	later for efficiency. The dirt_l/_r arrays track left/right dirty
 *	in a given area for redraw. Providing we are sensible when we force
 *	redraw this is reasonably efficient.
 *
 *	On character mapped direct displays we'll do something similar but
 *	of course can avoid the delayed update if we have buffer readbacks
 */

static void text_putc(uint_fast8_t y, uint_fast8_t x, char c)
{
	uint8_t *p = vidmem + y * (screen.right + 1) + x;
	/* Deal with simple clip cases right and down only */
	if (x > clip.right || y > clip.bottom)
		return;
	if (*p != c) {
		*p = c;
		/* Update the dirty status */
		if (dirt_l[y] > x)
			dirt_l[y] = x;
		if (dirt_r[y] < x)
			dirt_r[y] = x;
	}
}

/* For convenience clip left and right, but this is the only clip this internal
   routine does itself */
static uint_fast8_t text_putsn(uint_fast8_t y, uint_fast8_t x, const char *s, const char *e)
{
	while (s != e && *s && x <= clip.right) {
		if (*s == 0xFF) {
			s++;
			if (*s != 0xFF) {
				s ++;
				continue;
			}
		}
		if (x >= clip.left)
			text_putc(y, x, *s);
		x++;
		s++;
	}
	return x;
}

static uint_fast8_t text_puts(uint_fast8_t y, uint_fast8_t x, const char *s)
{
	return text_putsn(y, x, s, NULL);
}

static void hline(uint_fast8_t y, uint_fast8_t l, uint_fast8_t r, char c)
{
	if (r < l)
		return;
	/* For speed inline this so we don't keep changing dirt */
	memset(vidmem + y * (screen.right + 1) + l, c, r - l + 1);
	if (dirt_l[y] > l)
		dirt_l[y] = l;
	if (dirt_r[y] < r)
		dirt_r[y] = r;
}

static void clear_rect(struct utk_rect *r, char c)
{
	uint_fast8_t x = r->left;
	uint_fast8_t xr = r->right;
	uint_fast8_t y = r->top;
	uint_fast8_t yr = r->bottom;

	while (y <= yr)
		hline(y++, x, xr, c);
}

/*
 *	Actual higher level routines with clipping
 */

coord_t utk_putsn(coord_t y, coord_t x, const char *p, const char *e)
{
	if (y < clip.top)
		return x;
	if (y > clip.bottom)
		return x;
	/* text_puts clips left right itself */
	return text_putsn(y, x, p, e);
}

void utk_clear_rect(struct utk_rect *r, char c)
{
	struct utk_rect tmp;
	clip_save(&tmp);
	clip_intersect(r);
	clear_rect(r, c);
	clip_set(&tmp);
}

static void clip_hline(unsigned y, unsigned l, unsigned r, char c)
{
	if (y < clip.top || y > clip.bottom)
		return;
	if (l < clip.left)
		l = clip.left;
	if (r > clip.right)
		r = clip.right;
	hline(y, l, r, c);
}

static void clip_putc(unsigned y, unsigned x, char c)
{
	if (y < clip.top || y > clip.bottom)
		return;
	if (x < clip.left || x > clip.right)
		return;
	text_putc(y, x, c);
}

static void utk_do_menu(void)
{
	uint_fast8_t x = 0;
	if (win_top) {
		struct utk_menu *m = win_top->menu;
		while (m) {
			x = text_puts(0, x, m->title);
			x = text_puts(0, x, "_");
			m = m->next;
		}
	}
	hline(0, x, screen.right, '_');
}

void utk_menu(void)
{
	clip_set(&screen);
	utk_do_menu();
}

/*
 *	Fill in a rectangle of the background. For speed we work with the
 *	clipping rectange directly
 */
void utk_fill_background(void)
{
	clip_set(&damage);
	/* Fill the cliprect area */
	if (clip.top == 0) {
		utk_do_menu();
		clip.top++;
		clear_rect(&clip, ':');
		clip.top--;
	} else
		clear_rect(&clip, ':');
}

void utk_box(struct utk_rect *r)
{
	uint_fast8_t x = r->left;
	uint_fast8_t xb = r->right;
	uint_fast8_t y = r->top;
	uint_fast8_t yb = r->bottom;

	clip_putc(y, x, '+');
	clip_hline(y, x + 1, xb - 1, '-');
	clip_putc(y, xb, '+');
	while (y < yb) {
		clip_putc(y, x, '|');
		clip_hline(y, x + 1, xb - 1, ' ');
		clip_putc(y, xb, '|');
		y++;
	}
	clip_putc(y, x, '+');
	clip_hline(y, x + 1, xb - 1, '-');
	clip_putc(y, xb, '+');
}

/*
 *	Draw a window. The clip rectangle is the window or some subset of it
 */
void utk_win_base(struct utk_window *w)
{
	uint_fast8_t x;
	uint_fast8_t y;
	uint_fast8_t xr;
	uint_fast8_t yb;

	/* Top decoration */
	clip_set(&w->c.w.rect);
	/* Make sure the working area is on screen and needs redraw */
	clip_intersect(&damage);

	/* Set the window body clear. Could optimize this smaller perhaps
	   but need to take care of clip_intersect of screen effects and the
	   like */
	clear_rect(&clip, ' ');

	y = w->c.w.rect.top;
	x = w->c.w.rect.left;
	xr = w->c.w.rect.right;
	yb = w->c.w.rect.bottom;
	x = text_puts(y, x, "[+]=");
	x = text_puts(y, x, w->title);
	clip_hline(y, x, xr, '=');

	/* Right hand bar (for now don't worry about value to show */
	y = w->c.w.rect.top + 1;
	while (y <= yb) {
		clip_putc(y, xr, '#');
		y++;
	}
	/* Now the bottom and resize corner - again value to do */
	x = w->c.w.rect.left;
	clip_hline(yb, x, xr - 1, '#');
	clip_putc(yb, xr, '/');

}

/*
 * Render a widget. The cliprect is the visible window body area and we
 * need to stay within this. TODO: bias with scroll bars etc. Render any
 * children as we go.
 */
void utk_render_widget(struct utk_widget *w)
{
}

void utk_win_delete(struct utk_window *w)
{
	if (ev_win == w)
		ev_win = NULL;
}

static struct termios saved_term, term;

static void cleanup(int sig)
{
	tcsetattr(0, TCSADRAIN, &saved_term);
	exit(1);
}

static void exit_cleanup(void)
{
	tcsetattr(0, TCSADRAIN, &saved_term);
}

void utk_init(void)
{
	uint_fast8_t h = screen.bottom + 1;
	/* Until it's all tided up use malloc */
	dirt_l = malloc(2 * h + (screen.right + 1) * h);
	dirt_r = dirt_l + h;
	vidmem = dirt_r + h;
	memset(dirt_l, 0xFF, h);
	memset(dirt_r, 0x00, h);
	if (tcgetattr(0, &term) == 0) {
		saved_term = term;
		atexit(exit_cleanup);
		signal(SIGINT, cleanup);
		signal(SIGQUIT, cleanup);
		signal(SIGPIPE, cleanup);
		term.c_lflag &= ~(ICANON | ECHO);
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 0;
		term.c_cc[VINTR] = 0;
		term.c_cc[VSUSP] = 0;
		term.c_cc[VSTOP] = 0;
		tcsetattr(0, TCSADRAIN, &term);
	}
	printf("\033[0;0H\033[J");
	cursor_x = screen.right / 2;
	cursor_y = screen.bottom / 2;
	utk_cap.screen_height = h;
	utk_cap.screen_width = screen.right + 1;
}


void utk_exit(void)
{
	printf("\033[0;0H\033[J");
}

/*
 *	ASCII mode menu is a bit hackish but will do for nwo
 */

static uint_fast8_t menu_shown;
static uint_fast8_t menu_num;
static uint_fast8_t menu_row;
static uint_fast8_t menu_xbar;
static struct utk_menu *menu_ptr;
static struct utk_rect menu_rect;

/* Assume it fits and we don't have to scroll up and down the screen */
static void utk_menu_show(void)
{
	const char *p = menu_ptr->items;
	uint_fast8_t y = 1;

	menu_xbar = cursor_x;

	menu_rect.top = 1;
	menu_rect.bottom = 1 + menu_ptr->dropheight;
	menu_rect.left = cursor_x;
	menu_rect.right = menu_rect.left + menu_ptr->dropwidth;
	/* Slide it across to fit if need be */
	if (menu_rect.right > screen.right) {
		menu_rect.left -= menu_rect.right - screen.right;
		menu_rect.right = screen.right;
	}
	clip_set(&menu_rect);
	clear_rect(&menu_rect, ' ');
	while(p) {
		char *np = strchr(p, '/');
		text_putsn(y++, menu_rect.left + 1, p, np);
		if (np)
			np++;
		p = np;
	}
	cursor_y = menu_rect.top;
	cursor_x = menu_rect.left + 1;
	menu_shown = 1;
}

static void utk_menu_hide(void)
{
	damage_set(&menu_rect);
	menu_shown = 0;
	cursor_y = 0;
	cursor_x = menu_xbar;
	ui_redraw();
}

/* For now assume the menu actually fits. We need to scroll along it if not
   but that's for later */
static void utk_menu_right(void)
{
	struct utk_menu *p = menu_ptr->next;

	if (menu_shown)
		utk_menu_hide();
	if (p == NULL) {
		menu_ptr = win_top->menu;
		menu_num = 0;
		cursor_x = 0;
	} else {
		menu_ptr = p;
		menu_num++;
		cursor_x += menu_ptr->width;
	}
	utk_menu_show();
}


static void utk_menu_up(void)
{
	if (cursor_y > 1)
		cursor_y--;
}

static void utk_menu_down(void)
{
	/* Bottom line is blank so not selectable */
	if (cursor_y < menu_rect.bottom - 1)
		cursor_y++;
}

/*
 *	Primitve input text only
 */

static struct utk_event event;
static unsigned ui_mode;
static struct utk_rect ui_rect;

#define UI_NORMAL	0
#define UI_MOVE		1	/* Window move */
#define UI_RESIZE	2	/* Window resize */
#define UI_GRAB		3	/* Widget use */
#define UI_MENU		4	/* Menu bar */

#undef CTRL
#define CTRL(x)		((x) & 31)

static uint_fast8_t translate_key(char c)
{
	event.type = 0;

	if (ui_mode == UI_MENU) {
		if (c == '\033') {
			utk_menu_hide();
			cursor_y = save_y;
			cursor_x = save_x;
			ui_mode = UI_NORMAL;
			return 1;
		}
		if (c == '\t')
			utk_menu_right();
		else if (c == '\n' || c == '\r') {
			utk_menu_hide();
			cursor_y = save_y;
			cursor_x = save_x;
			ui_mode = UI_NORMAL;
			event.type = EV_MENU;
			event.code = (menu_num << 8) | menu_row;
			return 1;
		} else if (c == CTRL('P'))
			utk_menu_up();
		else if (c == CTRL('N'))
			utk_menu_down();
		return 1;
	}
	/* We will make this change when in a widget so that you have to
	   leave a text widget for the normal key behaviour return */
	if (c == CTRL('P')) {
		if (cursor_y)
			cursor_y--;
		return 1;
	}
	if (c == CTRL('N')) {
		if (cursor_y < screen.bottom)
			cursor_y++;
		return 1;
	}
	if (c == CTRL('B')) {
		if (cursor_x)
			cursor_x--;
		return 1;
	}
	if (c == CTRL('F')) {
		if (cursor_x < screen.right)
			cursor_x++;
		return 1;
	}
	if (c == '\033') {
		if (ui_mode) {
			ui_mode = 0;
			return 1;
		}
		menu_ptr = win_top->menu;
		if (menu_ptr == 0)
			return 1;
		/* Enter menu mode */
		save_y = cursor_y;
		save_x = cursor_x;
		cursor_y = 0;
		cursor_x = 0;
		ui_mode = UI_MENU;
		utk_menu_show();
		return 1;
	}
	if (c == '\n' || c == '\r') {
		if (ui_mode == UI_MOVE) {
			if (cursor_y == 0 || ev_win == NULL) {
				ui_mode = UI_NORMAL;
				return 1;
			}
			ui_rect.left += cursor_x;
			ui_rect.right += cursor_x;
			ui_rect.top += cursor_y;
			ui_rect.bottom += cursor_y;
			ui_mode = UI_NORMAL;
			ui_win_adjust(ev_win, &ui_rect);
			event.type = EV_MOVE;
			return 1;
			
		}
		if (ui_mode == UI_RESIZE) {
			ui_rect.bottom = cursor_y;
			ui_rect.right = cursor_x;
			if (ev_win == NULL || ui_rect.bottom < ui_rect.top + 2 ||
				ui_rect.right < ui_rect.left + 5) {
				ui_mode = UI_NORMAL;
				return 1;
			}
			ui_win_adjust(ev_win, &ui_rect);
			event.type = EV_RESIZE;
			ui_mode = UI_NORMAL;
			return 1;
		}
		if (event.window && cursor_y == event.window->c.w.rect.top) {
			if (cursor_x == event.window->c.w.rect.left + 1) {
				event.type = EV_CLOSE;
				event.code = 0;
				return 1;
			}
			if (event.window == win_top) {
				ui_mode = UI_MOVE;
				/* Set up as a 0 based rectange of the right size */
				ev_win = win_top;
				ui_rect.left = 0;
				ui_rect.top = 0;
				ui_rect.right = win_top->c.w.rect.right - win_top->c.w.rect.left;
				ui_rect.bottom = win_top->c.w.rect.bottom - win_top->c.w.rect.top;
				cursor_x = win_top->c.w.rect.left;
				cursor_y = win_top->c.w.rect.top;
				return 1;
			}
			event.type = EV_SELECT;
			event.code = 0;
			ui_win_top(event.window);
			return 1;
		}
		if (event.window && cursor_y == event.window->c.w.rect.bottom &&
			cursor_x == event.window->c.w.rect.right) {
			ui_rect.top = event.window->c.w.rect.top;
			ui_rect.left = event.window->c.w.rect.left;
			ui_mode = UI_RESIZE;
			ev_win = event.window;
			ui_win_top(ev_win);
			return 1;
		}
		if (event.window && event.window != win_top)
			ui_win_top(event.window);
		/* TODO: window scroll bars, window resize, window move */
		event.type = EV_BUTTON;
		event.code = 1;	/* Mouse button 1 */
		return 1;
	}
	if (c == '\t') {
		if (win_top && win_top->under) {
			ui_win_back(win_top);
			event.type = EV_SELECT;
			event.window = win_top;
			event.code = 0;
			cursor_y = win_top->c.w.rect.top + 1;
			cursor_x = win_top->c.w.rect.left;
			return 1;
		}
		return 1;
	}
	if (c == CTRL('X')) {
		event.type = EV_QUIT;
		return 1;
	}
	return 0;
}

/* Assume only text console for now */
struct utk_event *utk_event(void)
{
	char c;
	int n;

	utk_refresh();
	while((n = read(0, &c, 1)) == 1) {
		event.y = cursor_y;
		event.x = cursor_x;
		event.window = utk_find_window(event.y, event.x);
		if (!translate_key(c)) {
			event.type = EV_KEY;
			event.code = c;
		}
		if (event.type) {
			return &event;
		}
		/* Hack for the moment TODO */
		printf("\033[%d;%dH", cursor_y + 1, cursor_x + 1);
		utk_refresh();
		fflush(stdout);
	}
	if (n == -1) {
		/* TODO : timeouts, polling mode */
	}
	event.type = EV_QUIT;
	return &event;
}

struct utk_cap utk_cap = {
	0,
	0,
	1, 1,		/* No snap to boundary */
	1, 1,		/* Co-ordinates are character sized */
	0,		/* No capabilies */
};
