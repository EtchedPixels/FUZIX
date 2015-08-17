#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>

#define Y 18
#define gotosleep() sleep(5)

void main()
{
	WINDOW *main_window_ptr;
	WINDOW *status_window_ptr;
	char c;
	int i = 0;

	initscr();

	main_window_ptr = newwin(20, 80, 0, 0);
	status_window_ptr = newwin(1, 80, 21, 0);

	while (i++ < (Y - 1))
		wprintw(main_window_ptr, "testing curses functions.\n");
	
	touchwin(main_window_ptr); wrefresh(main_window_ptr);

	gotosleep();
	
	wmove(main_window_ptr, 0 , 0);

	c = (char)winch(main_window_ptr);

	werase(status_window_ptr); wrefresh(status_window_ptr); 
	mvwprintw(status_window_ptr, 0, 0, "winch: char at (%d, %d) is %c\n", main_window_ptr->_curx, main_window_ptr->_cury, c);
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

	gotosleep();

	c = (char)mvwinch(main_window_ptr, 9, 2);
	werase(status_window_ptr); wrefresh(status_window_ptr);
        mvwprintw(status_window_ptr, 0, 0, "winch: char at (%d, %d) is %c\n", main_window_ptr->_curx, main_window_ptr->_cury, c);
	touchwin(status_window_ptr); wrefresh(status_window_ptr);
	gotosleep();
	
	werase(status_window_ptr); wrefresh(status_window_ptr);
	mvwprintw(status_window_ptr, 0, 0, "wclrtoeol");
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

        gotosleep();

	wmove(main_window_ptr, 0, 7); 
	wclrtoeol(main_window_ptr);
	touchwin(main_window_ptr);
	wrefresh(main_window_ptr);	

        gotosleep();

        werase(status_window_ptr); wrefresh(status_window_ptr);
        mvwprintw(status_window_ptr, 0, 0, "winsch");
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

        gotosleep();

        wmove(main_window_ptr, 6, 0);
        winsch(main_window_ptr, 't');
        touchwin(main_window_ptr);
        wrefresh(main_window_ptr);

        gotosleep();

	werase(status_window_ptr); wrefresh(status_window_ptr); 
	mvwprintw(status_window_ptr, 0, 0, "wdelch");
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

        gotosleep();

	wmove(main_window_ptr, 6, 0);
	wdelch(main_window_ptr);
	touchwin(main_window_ptr);
	wrefresh(main_window_ptr);

        gotosleep();

	werase(status_window_ptr); wrefresh(status_window_ptr);
	mvwprintw(status_window_ptr, 0, 0, "wdeleteln");
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

	gotosleep();

	wmove(main_window_ptr, 7, 0);
        wdeleteln(main_window_ptr);
	touchwin(main_window_ptr);
	wrefresh(main_window_ptr);

	gotosleep();

        werase(status_window_ptr); wrefresh(status_window_ptr); 
	mvwprintw(status_window_ptr, 0, 0, "winsertln");
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

        gotosleep();

        wmove(main_window_ptr, 9, 0);
	winsertln(main_window_ptr);
	touchwin(main_window_ptr);
	wrefresh(main_window_ptr);

	gotosleep();

        werase(status_window_ptr); wrefresh(status_window_ptr);
        mvwprintw(status_window_ptr, 0, 0, "wclrtobot");
        touchwin(status_window_ptr); wrefresh(status_window_ptr);

        gotosleep();

        wmove(main_window_ptr, 9, 0);
        wclrtobot(main_window_ptr);
        touchwin(main_window_ptr);
        wrefresh(main_window_ptr);

        gotosleep();

	delwin(main_window_ptr);
	delwin(status_window_ptr);
	endwin();
}
