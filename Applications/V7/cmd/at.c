/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

/*
 * at time mon day
 * at time wday
 * at time wday 'week'
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HOUR 100
#define HALFDAY	(12*HOUR)
#define DAY	(24*HOUR)
#define THISDAY "/usr/spool/at"

const char *days[] = {
	"sunday",
	"monday",
	"tuesday",
	"wednesday",
	"thursday",
	"friday",
	"saturday",
};

struct monstr {
	const char *mname;
	int mlen;
} months[] = {
	{
	"january", 31}, {
	"february", 28}, {
	"march", 31}, {
	"april", 30}, {
	"may", 31}, {
	"june", 30}, {
	"july", 31}, {
	"august", 31}, {
	"september", 30}, {
	"october", 31}, {
	"november", 30}, {
	"december", 31}, {
0, 0},};

char fname[100];
int utimev;			/* requested time in grains */
int now;			/* when is it */
int uday;			/* day of year to be done */
int uyear;			/* year */
int today;			/* day of year today */
FILE *file;
FILE *ifile;

void onintr(int sig)
{
	unlink(fname);
	exit(1);
}

char *prefix(char *begin, char *full)
{
	int c;
	while (c = *begin++) {
		if (isupper(c))
			c = tolower(c);
		if (*full != c)
			return (0);
		else
			full++;
	}
	return (full);
}

int makeutime(char *pp)
{
	register int val;
	register char *p;

	/* p points to a user time */
	p = pp;
	val = 0;
	while (isdigit(*p)) {
		val = val * 10 + (*p++ - '0');
	}
	if (p - pp < 3)
		val *= HOUR;

	for (;;) {
		switch (*p) {

		case ':':
			++p;
			if (isdigit(*p)) {
				if (isdigit(p[1])) {
					val += (10 * *p + p[1] - 11 * '0');
					p += 2;
					continue;
				}
			}
			fprintf(stderr, "at: bad time format:\n");
			exit(1);

		case 'A':
		case 'a':
			if (val >= HALFDAY + HOUR)
				val = DAY + 1;	/* illegal */
			if (val >= HALFDAY && val < (HALFDAY + HOUR))
				val -= HALFDAY;
			break;

		case 'P':
		case 'p':
			if (val >= HALFDAY + HOUR)
				val = DAY + 1;	/* illegal */
			if (val < HALFDAY)
				val += HALFDAY;
			break;

		case 'n':
		case 'N':
			val = HALFDAY;
			break;

		case 'M':
		case 'm':
			val = 0;
			break;


		case '\0':
		case ' ':
			/* 24 hour time */
			if (val == DAY)
				val -= DAY;
			break;

		default:
			fprintf(stderr, "at: bad time format\n");
			exit(1);

		}
		break;
	}
	if (val < 0 || val >= DAY) {
		fprintf(stderr, "at: time out of range\n");
		exit(1);
	}
	if (val % HOUR >= 60) {
		fprintf(stderr, "at: illegal minute field\n");
		exit(1);
	}
	utimev = val;
}


int makeuday(int argc, const char *argv[])
{
	/* the presumption is that argv[2], argv[3] are either
	   month day OR weekday [week].  Returns either 2 or 3 as last
	   argument used */
	/* first of all, what's today */
	time_t tm;
	int found = -1;
	char **ps;
	struct tm *detail;
	struct monstr *pt;

	time(&tm);
	detail = localtime(&tm);
	uday = today = detail->tm_yday;
	uyear = detail->tm_year;
	now = detail->tm_hour * 100 + detail->tm_min;
	if (argc <= 2)
		return (1);
	/* is the next argument a month name ? */
	for (pt = months; pt->mname; pt++) {
		if (prefix(argv[2], pt->mname)) {
			if (found < 0)
				found = pt - months;
			else {
				fprintf(stderr, "at: ambiguous month\n");
				exit(1);
			}
		}
	}
	if (found >= 0) {
		if (argc <= 3)
			return (2);
		uday = atoi(argv[3]) - 1;
		if (uday < 0) {
			fprintf(stderr, "at: illegal day\n");
			exit(1);
		}
		while (--found >= 0)
			uday += months[found].mlen;
		if (detail->tm_year % 4 == 0 && uday > 59)
			uday += 1;
		return (3);
	}
	/* not a month, try day of week */
	found = -1;
	for (ps = days; ps < days + 7; ps++) {
		if (prefix(argv[2], *ps)) {
			if (found < 0)
				found = ps - days;
			else {
				fprintf(stderr,
					"at: ambiguous day of week\n");
				exit(1);
			}
		}
	}
	if (found < 0)
		return (1);
	/* find next day of this sort */
	uday = found - detail->tm_wday;
	if (uday <= 0)
		uday += 7;
	uday += today;
	if (argc > 3 && strcmp("week", argv[3]) == 0) {
		uday += 7;
		return (3);
	}
	return (2);
}


void filename(char *dir, int y, int d, int t)
{
	register int i;

	for (i = 0;; i += 53) {
		sprintf(fname, "%s/%02d.%03d.%04d.%02d", dir, y, d, t,
			(getpid() + i) % 100);
		if (access(fname, 0) == -1)
			return;
	}
}

int main(int argc, char *argv[])
{
	register int c;
	char pwbuf[100];
	FILE *pwfil;
	int larg;

	/* argv[1] is the user's time: e.g.,  3AM */
	/* argv[2] is a month name or day of week */
	/* argv[3] is day of month or 'week' */
	/* another argument might be an input file */
	if (argc < 2) {
		fprintf(stderr, "at: arg count\n");
		exit(1);
	}
	makeutime(argv[1]);
	larg = makeuday(argc, argv) + 1;
	if (uday == today && larg <= 2 && utimev <= now)
		uday++;
	c = uyear % 4 == 0 ? 366 : 365;
	if (uday >= c) {
		uday -= c;
		uyear++;
	}
	filename(THISDAY, uyear, uday, utimev);
	ifile = stdin;
	if (argc > larg)
		ifile = fopen(argv[larg], "r");
	if (ifile == NULL) {
		fprintf(stderr, "at: cannot open input: %s\n", argv[larg]);
		exit(1);
	}
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
	file = fopen(fname, "a");
	chmod(fname, 0644);
	if (file == NULL) {
		fprintf(stderr, "at: cannot open memo file\n");
		exit(1);
	}
	if ((pwfil = popen("pwd", "r")) == NULL) {
		fprintf(stderr, "at: can't execute pwd\n");
		exit(1);
	}
	fgets(pwbuf, 100, pwfil);
	pclose(pwfil);
	fprintf(file, "cd %s", pwbuf);
	if (environ) {
		char **ep = environ;
		while (*ep)
			fprintf(file, "%s\n", *ep++);
	}
	while ((c = getc(ifile)) != EOF) {
		putc(c, file);
	}
	exit(0);
}
