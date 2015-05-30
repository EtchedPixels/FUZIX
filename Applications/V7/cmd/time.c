/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied and shrunk a bit for FUZIX */

/* time command */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/times.h>

char quant[] = { 6, 10, 10, 6, 10, 6, 10, 10, 10 };

const char *pad = "000      ";
const char *sep = "\0\0.\0:\0:\0\0";
const char *nsep = "\0\0.\0 \0 \0\0";

void printt(const char *s, long a)
{
	char digit[9];
	register int i;
	char c;
	int nonzero;

	for (i = 0; i < 9; i++) {
		digit[i] = a % quant[i];
		a /= quant[i];
	}
	fputs(s, stderr);
	nonzero = 0;
	while (--i > 0) {
		c = digit[i] != 0 ? digit[i] + '0' :
		    nonzero ? '0' : pad[i];
		fputc(c, stderr);
		nonzero |= digit[i];
		c = nonzero ? sep[i] : nsep[i];
		fputc(c, stderr);
	}
	fputc('\n', stderr);
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
		fputs("Try again.\n", stderr);
		exit(1);
	}
	if (p == 0) {
		execvp(argv[1], (char**) &argv[1]);
		perror(argv[1]);
		_exit(1);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	times(&obuffer);
	while (wait(&status) != p)
		times(&obuffer);
	time(&after);
	if ((status & 0377) != 0)
		fputs("Command terminated abnormally.\n", stderr);
	times(&buffer);
	fprintf(stderr, "\n");
	printt("real", (after - before) * 60);
	printt("user", buffer.tms_cutime - obuffer.tms_cutime);
	printt("sys ", buffer.tms_cstime - obuffer.tms_cstime);
	exit(status >> 8);
}
