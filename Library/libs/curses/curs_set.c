#include <curses.h>
#include "curspriv.h"
#include <termcap.h>

extern char *vi, *ve, *vs;

void curs_set(int visibility)
{
	switch (visibility) {
		case 0:
			if (vi) tputs(vi, 1, outc);
			break;
		case 1:
			if (ve) tputs(ve, 1, outc);
		case 2:
			if (vs)
				tputs(vs, 1, outc);
			else if (ve)
				tputs(ve, 1, outc);
	}
}
