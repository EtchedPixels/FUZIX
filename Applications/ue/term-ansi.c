#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ue.h"

char str[MAXCOLS];		/* used by gotoxy and clrtoeol */

void gotoxy(int x, int y)
{
	sprintf(str,"%c[%03d;%03dH",0x1b,y,x);
	write(1, (void*)&str, 10);
	outxy.Y=y;outxy.X=x;
}

void clrtoeol(void)
{
	if (COLS-outxy.X > 0) {
		memset(str, ' ', COLS-outxy.X);
		write(1, str, COLS-outxy.X);
	}
	gotoxy(outxy.X,outxy.Y);
}


void clrtoeos(void)
{
	int i = outxy.Y;
	while(i++ <= ROWS){
		clrtoeol();
		gotoxy(1,i);
	}
}

void tty_init(void)
{
}
