static char *sccsid = "@(#)clear.c	4.1 (Berkeley) 10/1/80";
/* load me with -ltermlib */
/* #include <retrofit.h> on version 6 */
/*
 * clear - clear the screen
 */

#include <stdio.h>
#include <stdlib.h>
#include <termcap.h>

char	PC;

int tputchar(int c)
{
	putchar(c);
	return 0;
}

static char buf[1024];

int main(int argc, char *argv[])
{
	char *cp = getenv("TERM");
	char clbuf[20];
	char pcbuf[20];
	char *clbp = clbuf;
	char *pcbp = pcbuf;
	char *clear;
	char *pc;

	if (cp == (char *) 0)
		exit(1);
	if (tgetent(buf, cp) != 1)
		exit(1);
	pc = tgetstr("pc", &pcbp);
	if (pc)
		PC = *pc;
	clear = tgetstr("cl", &clbp);
	if (clear)
		tputs(clear, tgetnum("li"), tputchar);
	exit (clear != (char *) 0);
}
