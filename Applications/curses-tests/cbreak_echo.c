#include <unistd.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>

#define PW_LEN 256
#define NAME_LEN 256

int main() {

	char name[NAME_LEN];
	char password[PW_LEN];
	const char *real_password = "abcxyz";

	int i = 0;

	initscr();
	move(5, 10);
	printw("%s", "Please login:");
	move(7, 10);
	printw("%s", "User name: ");
	refresh();
	getstr(name);
	move(8, 10);
	printw("%s", "Password: ");
	refresh();

	cbreak();
	noecho();

	_tty.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &_tty);

	memset(password, '\0', sizeof(password));
	while (i < PW_LEN) {
		password[i] = getch();
		if (password[i] == '\r'| password[i] == '\n') break;
		move(8, 20 + i);
		addch('*');
		refresh();
		i++;
	}


	echo();
	nocbreak();
	move(11, 10);

	if (strncmp(real_password, password, strlen(real_password)) == 0)
		printw("%s", "Correct");
	else printw("%s", "Wrong");
	printw("%s", " password");
	refresh();

	sleep(2);
	endwin();
	exit(EXIT_SUCCESS);
}

