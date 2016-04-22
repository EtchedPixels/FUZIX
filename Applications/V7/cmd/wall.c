/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmp.h>
#define	USERS	50

char mesg[3000];
char buf[BUFSIZ];
int msize;
struct utmp utmp[USERS];

void sendmes(char *tty)
{
	register int i;
	char t[50];
	FILE *f;

	i = fork();
	if (i == -1) {
		fprintf(stderr, "Try again\n");
		return;
	}
	if (i)
		return;
	strcpy(t, "/dev/");
	strcat(t, tty);

	/* FIXME: needs to use unix I/O and O_NDELAY, plus set an alarm */
	if ((f = fopen(t, "w")) == NULL) {
		fprintf(stderr, "cannot open %s\n", t);
		exit(1);
	}
	setbuf(f, buf);
	fprintf(f, "Broadcast Message ...\n\n");
	fwrite(mesg, msize, 1, f);
	exit(0);
}

int main(int argc, const char *argv[])
{
	register int i;
	register struct utmp *p;
	FILE *f;

	if ((f = fopen("/var/run/utmp", "r")) == NULL) {
		fprintf(stderr, "Cannot open /var/run/utmp\n");
		exit(1);
	}
	fread((char *) utmp, sizeof(struct utmp), USERS, f);
	fclose(f);
	f = stdin;
	if (argc >= 2) {
		if ((f = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr, "Cannot open %s\n", argv[1]);
			exit(1);
		}
	}
	while ((i = getc(f)) != EOF)
		mesg[msize++] = i;
	fclose(f);
	for (i = 0; i < USERS; i++) {
		p = &utmp[i];
		if (p->ut_user[0] == 0)
			continue;
		sleep(1);
		sendmes(p->ut_line);
	}
	exit(0);
}
