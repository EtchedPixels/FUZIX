#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <termcap.h>
#include "ue.h"

static char *t_go;
static char *t_clreol;
static char *t_clreos;

/* str is either a space buffer for clearing or the output queue for
   termios, but never both at once */

static char str[MAXCOLS];
static char *ptr;

int outbuf(int c)
{
	*ptr++ = c;
}

void tputs_buf(char *p, int n)
{
	ptr = str;
	tputs(p, n, outbuf);
	write(1, str, ptr - str);
}

void gotoxy(int x, int y)
{
	tputs_buf(tgoto(t_go, x - 1, y - 1), 1);
	outxy.Y=y;
	outxy.X=x;
}

void clrtoeol(void)
{
	if (*t_clreol)
		tputs_buf(t_clreol, 1);
	else {
		int i=0;
		if (COLS-outxy.X > 0) {
			memset(str, ' ', COLS-outxy.X);
			write(1, str, COLS - outxy.X);
		}
	}
	gotoxy(outxy.X,outxy.Y);
}

void clrtoeos(void)
{
	if (*t_clreos)
		tputs_buf(t_clreos, ROWS-outxy.Y);
	else {
		int i = outxy.Y;
		while(i++ <= ROWS) {
			clrtoeol();
			gotoxy(1,i);
		}
	}
}

static int ival[3];

static char *tnext(char *p)
{
	return p + strlen(p) + 1;
}

void tty_init(void)
{
	int fd[2];
	pid_t pid;
	int l;

	if (pipe(fd) < 0) {
		perror("pipe");
		exit(1);
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(1);
	}

	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		execl("/usr/lib/tchelp", "tchelp", "li#co#cm$ce$cd$cl$", NULL);
		perror("tchelp");
		_exit(1);
	}
	close(fd[1]);
	if (read(fd[0], ival, 3 * sizeof(int)) != 3 * sizeof(int)) {
		perror("read");
		exit(1);
	}
	ival[0] -= 2 * sizeof(int);
	t_go = sbrk((ival[0] + 3) & ~3);
	if (t_go == (void *) -1) {
		perror("sbrk");
		exit(1);
	}
	if ((l = read(fd[0], t_go, ival[0])) != ival[0]) {
		if (l < 0)
			perror("read");
		else
			write(2, "ue: short read from tchelp.\n", 28);
		exit(1);
	}
	close(fd[0]);
	t_clreol = tnext(t_go);
	t_clreos = tnext(t_clreol);
	if (*t_clreos == 0)	/* No clr eos - try for clr/home */
		t_clreos++;	/* cl cap if present */
}
