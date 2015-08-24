#include <string.h>
#include <curses.h>
#include "curspriv.h"

/* These functions are expensive so keep them in their own files and
   don't suck them in for anything else.
   
   Really we should probably use the internal snprintf helpers and
   remove the length limit and buffer but that's a FIXME */

char __printscanbuf[513];	/* buffer used during I/O */
