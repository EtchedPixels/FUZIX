/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied and shrunk a bit for FUZIX */

/* time command */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/times.h>

static char quant[] = { 6, 10, 10, 6, 10, 6, 10, 10, 10 };

static const char *pad = "000      ";
static const char *sep = "\0\0.\0:\0:\0\0";
static const char *nsep = "\0\0.\0 \0 \0\0";

static int ticks;

static char buf[256];
static char *bp = buf;

#define writec(c)	*bp++ = (c)

static void writes(const char *s)
{
	write(2, s, strlen(s));
}

static void printt(const char *s, long a)
{
	char digit[9];
	register int i;
	char c;
	int nonzero;

	for (i = 0; i < 9; i++) {
		digit[i] = a % quant[i];
		a /= quant[i];
	}
	while(*s)
		writec(*s++);
	nonzero = 0;
	while (--i > 0) {
		c = digit[i] != 0 ? digit[i] + '0' :
		    nonzero ? '0' : pad[i];
		if (c) writec(c);
		nonzero |= digit[i];
		c = nonzero ? sep[i] : nsep[i];
		if (c) writec(c);
	}
	writec('\n');
}

void printc(const char *s, long a)
{
	a *= 60;
	a /= ticks;
	printt(s, a);
}

int main(int argc, const char *argv[])
{
	struct tms buffer, obuffer;
	int status;
	register int p;
	time_t before, after;

	if (argc <= 1)
		exit(0);
	time(&before);
	p = fork();
	if (p == -1) {
		writes("Try again.\n");
		exit(1);
	}
	if (p == 0) {
		execvp(argv[1], (char**) &argv[1]);
		perror(argv[1]);
		_exit(1);
	}

	ticks = sysconf(_SC_CLK_TCK);

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	times(&obuffer);
	while (wait(&status) != p)
		times(&obuffer);
	time(&after);
	if ((status & 0377) != 0)
		writes("Command terminated abnormally.\n");
	times(&buffer);

	writec('\n');
	printt("real", (after - before) * 60);
	printc("user", buffer.tms_cutime - obuffer.tms_cutime);
	printc("sys ", buffer.tms_cstime - obuffer.tms_cstime);
	writec(0);
	writes(buf);
	exit(status >> 8);
}
