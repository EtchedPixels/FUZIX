/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied and defloated for FUZIX */

/*
 * acct [ -w wtmp ] [ -d ] [ -p ] [ people ]
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <utmp.h>
#include <sys/types.h>

#define	TSIZE	33
#define	USIZE	200
struct utmp ibuf;

struct ubuf {
	char uname[8];
	long utime;
} ubuf[USIZE];

struct tbuf {
	struct ubuf *userp;
	long ttime;
} tbuf[TSIZE];

const char *wtmp;
int pflag, byday;
long dtime;
time_t midnight;
long lastime;
int pcount;
char **pptr;

const long day = 86400L;
const long dayandahalf = 86400L + 86400L / 2;


void update(struct tbuf *tp, int f)
{
	int j;
	struct ubuf *up;
	long t, t1;

	if (f)
		t = midnight;
	else
		t = ibuf.ut_time;
	if (tp->userp) {
		t1 = t - tp->ttime;
		if (t1 > 0 && t1 < dayandahalf)
			tp->userp->utime += t1;
	}
	tp->ttime = t;
	if (f)
		return;
	if (ibuf.ut_user[0] == '\0') {
		tp->userp = 0;
		return;
	}
	for (up = ubuf; up < &ubuf[USIZE]; up++) {
		if (up->uname[0] == '\0')
			break;
		for (j = 0; j < 8 && up->uname[j] == ibuf.ut_user[j]; j++);
		if (j >= 8)
			break;
	}
	for (j = 0; j < 8; j++)
		up->uname[j] = ibuf.ut_user[j];
	tp->userp = up;
}

int among(int i)
{
	register int j, k;
	register char *p;

	if (pcount == 0)
		return (1);
	for (j = 0; j < pcount; j++) {
		p = pptr[j];
		for (k = 0; k < 8; k++) {
			if (*p == ubuf[i].uname[k]) {
				if (*p++ == '\0')
					return (1);
			} else
				break;
		}
	}
	return (0);
}

void newday(void)
{
#if 0
	time_t ttime;
	struct timeb tb;

	time(&ttime);
/* FIXME: do modern style TZ */
	if (midnight == 0) {
		midnight = 60 * (long) tb.timezone;
		if (localtime(&ttime)->tm_isdst)
			midnight -= 3600;
	}
/* END */
#endif
	while (midnight <= ibuf.ut_time)
		midnight += day;
}

void pdate(void)
{
	time_t x;

	if (byday == 0)
		return;
	x = midnight - 1;
	printf("%.6s", ctime(&x) + 4);
}

void printdeci2(long v, int div)
{
	int r = ((int) (v / div)) % 100;
	if (r)
		printf(".%02d", r);
}

void print(void)
{
	int i;
	long ttime, t;

	ttime = 0;
	for (i = 0; i < USIZE; i++) {
		if (!among(i))
			continue;
		t = ubuf[i].utime;
		if (t > 0)
			ttime += t;
		if (pflag && ubuf[i].utime > 0) {
			printf("\t%-8.8s%6ld\n",
			       ubuf[i].uname, ubuf[i].utime / 3600L);
			printdeci2(ubuf[i].utime, 36);
		}
	}
	if (ttime > 0) {
		pdate();
		printf("\ttotal%9ld\n", ttime / 3600L);
		printdeci2(ttime, 36);
	}
}

void upall(int f)
{
	register struct tbuf *tp;

	for (tp = tbuf; tp < &tbuf[TSIZE]; tp++)
		update(tp, f);
}

void loop(void)
{
	register int i;
	register struct tbuf *tp;
	register struct ubuf *up;

	if (ibuf.ut_line[0] == '|') {
		dtime = ibuf.ut_time;
		return;
	}
	if (ibuf.ut_line[0] == '}') {
		if (dtime == 0)
			return;
		for (tp = tbuf; tp < &tbuf[TSIZE]; tp++)
			tp->ttime += ibuf.ut_time - dtime;
		dtime = 0;
		return;
	}
	if (lastime > ibuf.ut_time || lastime + dayandahalf < ibuf.ut_time)
		midnight = 0;
	if (midnight == 0)
		newday();
	lastime = ibuf.ut_time;
	if (byday && ibuf.ut_time > midnight) {
		upall(1);
		print();
		newday();
		for (up = ubuf; up < &ubuf[USIZE]; up++)
			up->utime = 0;
	}
	if (ibuf.ut_line[0] == '~') {
		ibuf.ut_user[0] = '\0';
		upall(0);
		return;
	}
	if (ibuf.ut_line[0] == 't')
		i = (ibuf.ut_line[3] - '0') * 10 + (ibuf.ut_line[4] - '0');
	else
		i = TSIZE - 1;
	if (i < 0 || i >= TSIZE)
		i = TSIZE - 1;
	tp = &tbuf[i];
	update(tp, 0);
}

int main(int argc, char *argv[])
{
	int c, fl;
	register int i;
	FILE *wf;

	wtmp = "/usr/adm/wtmp";
	while (--argc > 0 && **++argv == '-')
		switch (*++*argv) {
		case 'd':
			byday++;
			continue;

		case 'w':
			if (--argc > 0)
				wtmp = *++argv;
			continue;

		case 'p':
			pflag++;
			continue;
		}
	pcount = argc;
	pptr = argv;
	if ((wf = fopen(wtmp, "r")) == NULL) {
		printf("No %s\n", wtmp);
		exit(1);
	}
	for (;;) {
		if (fread((char *) &ibuf, sizeof(ibuf), 1, wf) != 1)
			break;
		fl = 0;
		for (i = 0; i < 8; i++) {
			c = ibuf.ut_user[i];
			if (isdigit(c) || isalpha(c)) {
				if (fl)
					goto skip;
				continue;
			}
			if (c == ' ' || c == '\0') {
				fl++;
				ibuf.ut_user[i] = '\0';
			} else
				goto skip;
		}
		loop();
	      skip:;
	}
	ibuf.ut_user[0] = '\0';
	ibuf.ut_line[0] = '~';
	time(&ibuf.ut_time);
	loop();
	print();
	exit(0);
}
