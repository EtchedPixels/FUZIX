#include <curses.h>
#include "curspriv.h"

/****************************************************************/
/* Mvcur(oldy,oldx,newy,newx) the display cursor to <newy,newx>	*/
/****************************************************************/

int mvcur(int oldx, int oldy, int newy, int newx)
{
  if ((newy >= LINES) || (newx >= COLS) || (newy < 0) || (newx < 0))
	return(ERR);
  poscur(newy, newx);
  _cursvar.cursrow = newy;
  _cursvar.curscol = newx;
  return(OK);
}
