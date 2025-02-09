/*
* s - a screen editor
*
*
* Source files:
*
*		command handler:
*	address.c	- process addresses
*	commands.c	- commands without an address
*	keyboard.c	- read commands
*	lib.c		- library of C procedures
*	operator.c	- operators c, d and y
*	s.c		- this file; contains the main program
*	s.h		- macro definitions
*	yank.c		- the yank buffer
*
*		buffer module:
*	Bman.c		- buffer manager
*	buffer.c	- data structure for the buffer
*
*		screen module:
*	Sman.c		- screen manager
*	screen.c	- terminal-specific procedures
*
*
* External procedure calls:
*
*	address(n, c, op)				.. file address.c
*	int n;
*	char c, op;
*		Set the buffer's cursor according to the count n,
*		the cursor movement command c and the operation op.
*
*	b_getcur(line_ptr, pos_ptr)			.. file Bman.c
*	int *line_ptr, *pos_ptr;
*		Return the line and position of the cursor.
*
*	b_init()					.. file Bman.c
*		Initialize the buffer module.
*
*	k_donext(command)				.. file keyboard.c
*	char *command;
*		Arrange for the given edit command to be executed next.
*
*	int k_getch()					.. file keyboard.c
*		Return the next character of the command.
*
*	k_init()					.. file keyboard.c
*		Initialize the keyboard manager.
*
*	k_newcmd()					.. file keyboard.c
*		Prepare for reading a new command.
*
*	operator(op, line, pos)				.. file operator.c
*	char op;
*	int line, pos;
*		Apply op = 'c', 'd' or 'y' using the indicated buffer
*		location and the cursor location.
*
*	s_init()					.. file Sman.c
*		Initialize the screen module.
*
*	s_refresh()					.. file Sman.c
*		Bring the screen up to date with the buffer.
*
*	int simp_cmd(n, c)				.. file commands.c
*	int n;
*	char c;
*		Apply commands without addresses; tell if successful.
*
*
* System Dependencies:
*
*	To move this editor to a non-UNIX operating system, the functions
*	k_setraw() and k_setcooked() in file keyboard.c must be changed.
*	These functions flip the terminal driver to and from noecho-raw mode.
*
*	To operate this editor on a non-ANSI standard video terminal, or one
*	without "autowrap", requires modification of the file screen.c.
*/

#include "s.h"

extern void b_init(), k_init(), s_init(), k_donext(), s_refresh();
extern void k_newcmd(), b_getcur(), address(), operator();
extern int fatal(), k_getch(), simp_cmd();
static int get_count();

int main(argc, argv)
int argc;
char *argv[];
{
	int count, count2, cur_line, cur_pos, new_line, new_pos;
	char c, op, cmd[MAXTEXT];

	if (argc != 2)
		fatal("usage: s file");
	b_init();
	k_init();
	s_init();
	/* do the command:  :e<space><file><return> */
	sprintf(cmd, ":e %s%c", argv[1], CR);
	k_donext(cmd);
	for ( ; ; ) {	/* loop over commands */
		/* prepare to get a new command */
		s_refresh();
		k_newcmd();
		c = k_getch();
		count = get_count(&c);
		/* for simple commands, move on to the next command */
		if (simp_cmd(count, c))
			continue;
		/* for c, d or y operators, get the second count */
		if (c == 'c' || c == 'd' || c == 'y') {
			op = c;
			c = k_getch();
			count2 = get_count(&c);
			if (count > 0 && count2 > 0)
				count *= count2;
			else
				count = max(count, count2);
		} else
			op = ' ';
		/* set the buffer's idea of the cursor to the new address */
		b_getcur(&cur_line, &cur_pos);
		address(count, c, op);
		/* check that cursor actually moved */
		b_getcur(&new_line, &new_pos);
		if (cur_line == new_line && cur_pos == new_pos)
			UNKNOWN;
		else if (op != ' ')
			operator(op, cur_line, cur_pos);
	}
	return(0);
}

/* get_count - determine a count in an edit command */
static int get_count(ch_ptr)
char *ch_ptr;
{
	int ch = *ch_ptr, count;

	if (isdigit(ch) && ch != '0') {	/* a count cannot start with zero */
		count = ch - '0';
		while (isdigit(ch = k_getch()))
			count = 10*count + ch - '0';
	} else
		count = 0;
	*ch_ptr = ch;
	return(count);
}
