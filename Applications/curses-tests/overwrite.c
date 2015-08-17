#include<stdio.h>
#include<curses.h>
#include<stdlib.h>
#include<unistd.h>

void main()
{
        WINDOW *w = newwin(5, 20, 5, 5);
	int i = 0;

	initscr();

	while (i++ < (LINES * COLS)/2)
		printw("a ");

	refresh();
	sleep(5);

	wprintw(w, "b b b b b b b b b b \nb b b b b b b ");
	
	touchwin(w);
	
	overwrite(w, stdscr);

	refresh();

	sleep(5);
	delwin(w);
	endwin();
}
