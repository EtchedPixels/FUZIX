#include <unistd.h>
#include "ue.h"

static char str[4] = "\x1bY";

void gotoxy(int x, int y){
	str[2] = y + 31;
	str[3] = x + 31;
	write(1, (void*)&str, 4);
	outxy.Y=y;outxy.X=x;
}

void clrtoeol(void)
{
	write(1, "\x1b" "K", 2);
	gotoxy(outxy.X,outxy.Y);
}

void clrtoeos(void)
{
	write(1, "\x1b" "J", 2);	/* clear to end of screen */
}

void tty_init(void)
{
}
