#include <string.h>
#include <curses.h>
#include "curspriv.h"

extern char __printscanbuf[513];	/* buffer used during I/O */

/****************************************************************/
/* Wprintw(win,fmt,args) does a printf() in window 'win'.	*/
/****************************************************************/
int wprintw(WINDOW *win, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vsnprintf(__printscanbuf, sizeof(__printscanbuf), fmt, args);
  if (waddstr(win, __printscanbuf) == ERR) return(ERR);
  return(strlen(__printscanbuf));
}

/****************************************************************/
/* Printw(fmt,args) does a printf() in stdscr.			*/
/****************************************************************/
int printw(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vsnprintf(__printscanbuf, sizeof(__printscanbuf), fmt, args);
  if (waddstr(stdscr, __printscanbuf) == ERR) return(ERR);
  return(strlen(__printscanbuf));
}				/* printw */

/****************************************************************/
/* Mvprintw(fmt,args) moves the stdscr cursor to a new posi-	*/
/* tion, then does a printf() in stdscr.			*/
/****************************************************************/
int mvprintw(int y, int x, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if (wmove(stdscr, y, x) == ERR) return(ERR);
  vsnprintf(__printscanbuf, sizeof(__printscanbuf), fmt, args);
  if (waddstr(stdscr, __printscanbuf) == ERR) return(ERR);
  return(strlen(__printscanbuf));
}

/****************************************************************/
/* Mvwprintw(win,fmt,args) moves the window 'win's cursor to	*/
/* A new position, then does a printf() in window 'win'.	*/
/****************************************************************/
int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if (wmove(win, y, x) == ERR) return(ERR);
  vsnprintf(__printscanbuf, sizeof(__printscanbuf), fmt, args);
  if (waddstr(win, __printscanbuf) == ERR) return(ERR);
  return(strlen(__printscanbuf));
}				/* mvwprintw */
