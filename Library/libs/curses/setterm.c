#include <curses.h>
#include "curspriv.h"

static void ttysetflags(void)
{
  _tty.c_iflag |= ICRNL | IXON;
  _tty.c_oflag |= OPOST | ONLCR;
  _tty.c_lflag |= ECHO | ICANON | IEXTEN | ISIG;
  _tty.c_cc[VEOF] = 4;

  if (_cursvar.rawmode) {
	_tty.c_iflag &= ~(ICRNL | IXON);
	_tty.c_oflag &= ~(OPOST);
	_tty.c_lflag &= ~(ICANON | IEXTEN | ISIG);
	_tty.c_cc[VMIN] = 1;
  }
  if (_cursvar.cbrkmode) {
	_tty.c_lflag &= ~(ICANON);
	_tty.c_cc[VMIN] = 1;
  }
  if (!_cursvar.echoit) {
	_tty.c_lflag &= ~(ECHO | ECHONL);
  }
  if (NONL) {
	_tty.c_iflag &= ~(ICRNL);
	_tty.c_oflag &= ~(ONLCR);
  }
  tcsetattr(0, TCSANOW, &_tty);
}				/* ttysetflags */

void raw(void)
{
  _cursvar.rawmode = TRUE;
  ttysetflags();
}				/* raw */

void noraw(void)
{
  _cursvar.rawmode = FALSE;
  ttysetflags();
}				/* noraw */

void echo(void)
{
  _cursvar.echoit = TRUE;
  ttysetflags();
}

void noecho(void)
{
  _cursvar.echoit = FALSE;
  ttysetflags();
}

void nl(void)
{
  NONL = FALSE;
  ttysetflags();
}				/* nl */

void nonl(void)
{
  NONL = TRUE;
  ttysetflags();
}				/* nonl */

void cbreak(void)
{
  _cursvar.cbrkmode = TRUE;
  ttysetflags();
}				/* cbreak */

void nocbreak(void)
{
  _cursvar.cbrkmode = FALSE;
  ttysetflags();
}				/* nocbreak */
