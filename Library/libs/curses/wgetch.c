#include <curses.h>
#include <stdio.h>
#include "curspriv.h"

int wgetch(WINDOW *win)
{
	int inp;

	if (_cursvar.echoit && !win->_scroll && (win->_flags & _FULLWIN)
		&& win->_curx == win->_maxx - 1 && win->_cury == win->_maxy - 1)
		return ERR;

	wrefresh(win);
	clearerr(stdin);
	inp = getchar();
	if (inp == EOF)
		return ERR;
	if (_cursvar.echoit) {
		mvwaddch(curscr, win->_cury + win->_begy,
			win->_curx + win->_begx, inp);
		waddch(win, inp);
	}
	return inp;
}
