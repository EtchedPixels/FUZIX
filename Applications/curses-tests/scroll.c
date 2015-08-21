#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int i = 0;

	initscr();

	scrollok(stdscr,TRUE);

	while(i < 250)
	{
		printw("%d - lots and lots of lines flowing down the terminal\n", i);
		++i;
		refresh();
	}

	getch();
	endwin();
	return 0;
}
