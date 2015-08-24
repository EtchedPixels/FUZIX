#include <curses.h>
#include "curspriv.h"
#include <termcap.h>

int endwin(void)
{
	extern char *me;
	
	//curs_set(1);
	refresh();

	poscur(LINES - 1, 0);

	tputs(me, 1, outc);

	delwin(stdscr);
	delwin(curscr);
	delwin(_cursvar.tmpwin);

        tcsetattr(1, TCSANOW, &_orig_tty);

	return (OK);
}
