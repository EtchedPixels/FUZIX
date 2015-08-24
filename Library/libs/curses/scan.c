#include <string.h>
#include <curses.h>
#include "curspriv.h"

extern char __printscanbuf[513];	/* buffer used during I/O */

/****************************************************************/
/* Wscanw(win,fmt,args) gets a string via window 'win', then	*/
/* Scans the string using format 'fmt' to extract the values	*/
/* And put them in the variables pointed to the arguments.	*/
/****************************************************************/
int wscanw(WINDOW *win, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  wrefresh(win);		/* set cursor */
  if (wgetstr(win, __printscanbuf) == ERR)	/* get string */
	return(ERR);
  return(vsscanf(__printscanbuf, fmt, args));
}				/* wscanw */

/****************************************************************/
/* Scanw(fmt,args) gets a string via stdscr, then scans the	*/
/* String using format 'fmt' to extract the values and put them	*/
/* In the variables pointed to the arguments.			*/
/****************************************************************/
int scanw(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  wrefresh(stdscr);		/* set cursor */
  if (wgetstr(stdscr, __printscanbuf) == ERR)	/* get string */
	return(ERR);
  return(vsscanf(__printscanbuf, fmt, args));
}				/* scanw */

/****************************************************************/
/* Mvscanw(y,x,fmt,args) moves stdscr's cursor to a new posi-	*/
/* Tion, then gets a string via stdscr and scans the string	*/
/* Using format 'fmt' to extract the values and put them in the	*/
/* Variables pointed to the arguments.				*/
/****************************************************************/
int mvscanw(int y, int x, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if (wmove(stdscr, y, x) == ERR) return(ERR);
  wrefresh(stdscr);		/* set cursor */
  if (wgetstr(stdscr, __printscanbuf) == ERR)	/* get string */
	return(ERR);
  return(vsscanf(__printscanbuf, fmt, args));
}				/* mvscanw */

/****************************************************************/
/* Mvwscanw(win,y,x,fmt,args) moves window 'win's cursor to a	*/
/* New position, then gets a string via 'win' and scans the	*/
/* String using format 'fmt' to extract the values and put them	*/
/* In the variables pointed to the arguments.			*/
/****************************************************************/
int mvwscanw(WINDOW *win, int y, int x, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if (wmove(win, y, x) == ERR) return(ERR);
  wrefresh(win);		/* set cursor */
  if (wgetstr(win, __printscanbuf) == ERR)	/* get string */
	return(ERR);
  return(vsscanf(__printscanbuf, fmt, args));
}				/* mvwscanw */
