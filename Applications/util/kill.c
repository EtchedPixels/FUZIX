/* Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Stripped of stdio usage Alan Cox, 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct sigentry 
{
	char *suffix;        /* suffix of textual signal name */
	int  no;             /* number of signal */
};

struct sigentry sigtab[]={
	{ "HUP",   1 },
	{ "INT",   2 },
	{ "QUIT",  3 },
	{ "ILL",   4 },
        { "TRAP",  5 },
	{ "ABRT",  6 },
	{ "IOT",   6 },
	{ "BUS",   7 },
	{ "FPE",   8 },
	{ "KILL",  9 },
	{ "USR1",  10},
	{ "SEGV",  11},
	{ "USR2",  12},
	{ "PIPE",  13},
	{ "ALRM",  14},
	{ "TERM",  15},
	{ "STKFLT",16},
	{ "CHLD",  17},
	{ "CONT",  18},
	{ "STOP",  19},
	{ "TSTP",  20},
	{ "TTIN",  21},
	{ "TTOU",  22},
	{ "URG"	,  23},
	{ "XCPU",  24},
	{ "XFSZ",  25},
	{ "VTALRM",26},
	{ "PROF",  27},
	{ "WINCH", 28},
	{ "IO",    29},
	{ "POLL",  29},
	{ "PWR",   30},
	{ "SYS",   31},
	{ "UNUSED",31},
	{ NULL, 0 }
};

int getsig(char *n)
{
	struct sigentry *s=sigtab;
	while (s->suffix) {
		if (!strcmp(n,s->suffix))
			return s->no;
		s = s + 1;
	}
	return -1;
}	

void diesig(void)
{
	write(2, "kill: unknown signal\n", 21);
	exit(1);
}

void printsigs(void)
{
	struct sigentry *s=sigtab;
	int col=0;
	while (s->suffix) {
		int l = strlen(s->suffix);
		const char *np = _itoa(s->no);
		write(2, "SIG", 3 );
		write(2, s->suffix, l);
		write(2, "       ", 7 - l);
		write(2, np, strlen(np));
		if (col++ & 1)
			write(2, "\n", 1);
		else
			write(2, "   ", 3);
		s=s+1;
	}
	exit(0);
}

int main(int argc, char *argv[])
{
    char *cp;
    int pid, sig = SIGTERM;

    if (argc < 2) {
	write(2, "usage: kill [-sig] pid ...\n", 27);
	return 0;
    }
    if (argv[1][0] == '-') {
	cp = &argv[1][1];
	if( *cp=='l' ) 
		printsigs();
	if (!strncmp(cp, "SIG", 3)){
		cp += 3;
		sig=getsig( cp );
		if( sig < 0 ) 
			diesig();
	}
	else {
	    sig = 0;
	    while (isdigit(*cp))
		sig = sig * 10 + *cp++ - '0';
	    if (*cp)
		    diesig();
	}
	argc--;
	argv++;
    }
    while (argc-- > 1) {
	cp = *++argv;
	pid = 0;
	while (isdigit(*cp))
	    pid = pid * 10 + *cp++ - '0';
	if (*cp) {
	    write(1, "kill: non-numeric pid\n", 22);
	    continue;
	}
	if (kill(pid, sig) < 0) {
	    perror(*argv);
	    return 1;
	}
    }
    return 0;
}
