/*
* keyboard.c - read commands
*
*
* Entry points:
*
*	k_donext(cmd)
*	char *cmd;
*		Arrange that cmd will be done next.
*
*	k_finish()
*		Close down the keyboard manager.
*
*	int k_getch()
*		Return the next character of the current command.
*
*	k_init()
*		Initialize the keyboard manager.
*
*	char k_lastcmd()
*		Return the first letter in the last command.
*
*	k_newcmd()
*		Prepare for reading a new command.
*
*	k_redo()
*		Redo the last buffer-change command.
*
*	int k_keyin()
*		Get a character from the keyboard.
*
*
* External procedure calls:
*
*	int b_changed()					.. file Bman.c
*		Tell if the buffer was changed by the last command.
*
*	b_newcmd(bit)					.. file Bman.c
*	int bit;
*		Inform the buffer module that a new command is starting and tell
*		whether it is from the keyboard (bit = 1) or not (bit = 0).
*
*	s_keyboard(bit)					.. file Sman.c
*	int bit;
*		Inform the screen module whether the next input character is
*		from the keyboard (bit = 1) or not (bit = 0).
*
*	s_savemsg(msg, count)				.. file Sman.c
*	char *msg;
*	int count;
*		Format msg and save it for the next screen refreshing.
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "s.h"

#define CMD_MAX 500		/* longest command that can be redone */

extern void s_savemsg(), s_keyboard(), b_newcmd();
extern int b_changed();
int k_keyin();
void k_setraw();
void k_setcooked();

static char
	change[CMD_MAX+2],	/* most recent buffer-change command */
	cmd_last,		/* first letter in the last command */
	command[CMD_MAX+2],	/* accumulates the current command */
	*cmd_ptr = command,	/* next location in command */
	pushed[CMD_MAX],	/* pushed-back command */
	*push_ptr = pushed;	/* next location in pushed */

/* k_donext - push a command back on the input stream */
void k_donext(cmd)
char *cmd;
{
	int cmd_size;
	char *s;

	cmd_size = strlen(cmd);
	if (push_ptr - pushed + cmd_size > CMD_MAX) {
		s_savemsg("Pushed commands are too long.", 0);
		UNKNOWN;
	} else if (cmd_size > 0) {
		/* copy cmd to pushed[] in reverse order */
		for (s = cmd + cmd_size - 1; s >= cmd; --s)
			*push_ptr++ = *s;
		s_keyboard(0);
	}
}

/* k_finish - close down the keyboard manager */
void k_finish()
{
	k_setcooked();
}

/* k_getch - get a character of the command */
int k_getch()
{
	int ch;

	/* get pushed character (preferably) or read keyboard */
	/* use logical AND operation with octal 0177 to strip the parity bit */
	ch = (push_ptr > pushed) ? *(--push_ptr) : k_keyin() & 0177;
	/* remember character if there is room */
	if (cmd_ptr <= command + CMD_MAX)
		*cmd_ptr++ = ch;
	s_keyboard(push_ptr == pushed);
	return(ch);
}

/* k_init - initialize the keyboard manager */
void k_init()
{
	k_setraw();
}

/* k_lastcmd - get first letter of the last command */
char k_lastcmd()
{
	return(cmd_last);
}

/* k_newcmd - start a new command */
void k_newcmd()
{
	char *s;

	*cmd_ptr = '\0';
	/* remember first letter of the old command */
	for (s = command; *s != '\0' && !isalpha(*s); ++s)
		;
	cmd_last = *s;
	/* if the old command changed the buffer, remember it */
	if (b_changed())
		strcpy(change, command);
	cmd_ptr = command;		/* prepare to collect the new command */
	b_newcmd(push_ptr == pushed);	/* mark buffer "unchanged" */
}

/* k_redo - redo the last buffer-change command */
void k_redo()
{
	if (strlen(change) > CMD_MAX) {
		s_savemsg("Cannot redo commands longer than %d characters.",
			CMD_MAX);
		change[0] = '\0';
	}
	if (change[0] == '\0')
		UNKNOWN;
	else
		k_donext(change);
}

/*	
* k_keyin - get a character from the keyboard
* Hide system dependent differences in keyboard input
*/

int k_keyin()
{
  char ch;
  read(0,&ch,1);
  return((int)ch);
}

static struct termios echoT;		/* Original */

/*	
* k_setcooked  - set terminal to cooked mode
* Assumes k_setraw() previously called
*/
void k_setcooked() {
  tcsetattr(0, TCSANOW, &echoT);
}

/*	
* k_setraw  - set terminal to raw mode
*/

void k_setraw()
{
  struct termios rawT;		/* Raw mode, no echo */

  /* Try to get the current termios setting */
  tcgetattr(0, &echoT);
  if (tcgetattr(0, &rawT) == -1)
    return;

  /* Set rawT up to be raw mode */
  rawT.c_lflag &= ~(ICANON | ECHO);
  rawT.c_lflag |= ISIG;
  rawT.c_iflag &= ~ICRNL;
  rawT.c_cc[VMIN] = 1;             /* Character-at-a-time input */
  rawT.c_cc[VTIME] = 0;            /* with blocking */
  tcsetattr(0, TCSANOW, &rawT);
  atexit(k_setcooked);
}
