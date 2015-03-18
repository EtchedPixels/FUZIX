/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * mesg -- set current tty to accept or
 *	forbid write permission.
 *
 *	mesg [y] [n]
 *		y allow messages
 *		n forbid messages
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>

struct stat sbuf;

char *tty;

void newmode(int m)
{
	if(chmod(tty,m)<0)
		err(1, "cannot change mode");
}

int main(int argc, char *argv[])
{
	int r=0;
	tty = ttyname(2);
	if(stat(tty, &sbuf) < 0) err(1, "cannot stat");
	if(argc < 2) {
		if(sbuf.st_mode & 02)
			fprintf(stderr,"is y\n");
		else {	r=1;
			fprintf(stderr,"is n\n");
		}
	} else	switch(*argv[1]) {
		case 'y':
			newmode(0622); break;

		case 'n':
			newmode(0600); r=1; break;

		default:
			errx(-1, "usage: mesg [y] [n]");
		}
	exit(r);
}

