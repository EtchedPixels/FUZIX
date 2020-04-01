/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * Type tty name
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	const char *p;

	p = ttyname(0);
	if (p == NULL)
		p = "not a tty";

	if(argc == 2 && !strcmp(argv[1], "-s"))
		;
	else {
		puts(p);
		putchar('\n');
	}
	exit(p? 0: 1);
}
