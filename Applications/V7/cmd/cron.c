/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

/* FIXME: hardcoded UID 1 assumption */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>

#define	LISTS	512

#define	EXACT	100
#define	ANY	101
#define	LIST	102
#define	RANGE	103
#define	EOS	104

char crontab[] = "/usr/lib/crontab";
time_t itime;
struct tm *loct;
int flag;
char *list;
unsigned listsize;


char *cmp(char *p, int v)
{
	register char *cp;

	cp = p;
	switch (*cp++) {

	case EXACT:
		if (*cp++ != v)
			flag++;
		return (cp);

	case ANY:
		return (cp);

	case LIST:
		while (*cp != LIST)
			if (*cp++ == v) {
				while (*cp++ != LIST);
				return (cp);
			}
		flag++;
		return (cp + 1);

	case RANGE:
		if (*cp > v || cp[1] < v)
			flag++;
		return (cp + 2);
	}
	if (cp[-1] != v)
		flag++;
	return (cp);
}

int number(int c)
{
	int n = 0;

	while (isdigit(c)) {
		n = n * 10 + c - '0';
		c = getchar();
	}
	ungetc(c, stdin);
	if (n >= 100)
		return (-1);
	return (n);
}


void slp(void)
{
	int i;
	time_t t;

	time(&t);
	i = itime - t;
	if (i > 0)
		sleep(i);
}

void ex(char *s)
{
	int st;

	if (fork()) {
		wait(&st);
		return;
	}
	if (fork())
		exit(0);
	freopen("/", "r", stdin);
	execl("/bin/sh", "sh", "-c", s, 0);
	exit(0);
}

void init(void)
{
	register int i, c;
	register char *cp;
	register char *ocp;
	register int n;

	freopen(crontab, "r", stdin);
	if (list) {
		free(list);
		list = realloc(list, LISTS);
	} else
		list = malloc(LISTS);
	listsize = LISTS;
	cp = list;

      loop:
	if (cp > list + listsize - 100) {
		char *olist;
		listsize += LISTS;
		olist = list;
		free(list);
		list = realloc(list, listsize);
		cp = list + (cp - olist);
	}
	ocp = cp;
	for (i = 0;; i++) {
		do
			c = getchar();
		while (c == ' ' || c == '\t');
		if (c == EOF || c == '\n')
			goto ignore;
		if (i == 5)
			break;
		if (c == '*') {
			*cp++ = ANY;
			continue;
		}
		if ((n = number(c)) < 0)
			goto ignore;
		c = getchar();
		if (c == ',')
			goto mlist;
		if (c == '-')
			goto mrange;
		if (c != '\t' && c != ' ')
			goto ignore;
		*cp++ = EXACT;
		*cp++ = n;
		continue;

	      mlist:
		*cp++ = LIST;
		*cp++ = n;
		do {
			if ((n = number(getchar())) < 0)
				goto ignore;
			*cp++ = n;
			c = getchar();
		} while (c == ',');
		if (c != '\t' && c != ' ')
			goto ignore;
		*cp++ = LIST;
		continue;

	      mrange:
		*cp++ = RANGE;
		*cp++ = n;
		if ((n = number(getchar())) < 0)
			goto ignore;
		c = getchar();
		if (c != '\t' && c != ' ')
			goto ignore;
		*cp++ = n;
	}
	while (c != '\n') {
		if (c == EOF)
			goto ignore;
		if (c == '%')
			c = '\n';
		*cp++ = c;
		c = getchar();
	}
	*cp++ = '\n';
	*cp++ = 0;
	goto loop;

      ignore:
	cp = ocp;
	while (c != '\n') {
		if (c == EOF) {
			*cp++ = EOS;
			*cp++ = EOS;
			fclose(stdin);
			return;
		}
		c = getchar();
	}
	goto loop;
}

int main(int argc, char *argv[])
{
	register char *cp;
	time_t filetime = 0;

	setuid(1);
	if (fork())
		exit(0);
	chdir("/");
	freopen(crontab, "r", stdin);
	freopen("/", "r", stdout);
	freopen("/", "r", stderr);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	time(&itime);
	itime -= localtime(&itime)->tm_sec;
	fclose(stdin);

	for (;; itime += 60, slp()) {
		struct stat cstat;

		if (stat(crontab, &cstat) == -1)
			continue;
		if (cstat.st_mtime > filetime) {
			filetime = cstat.st_mtime;
			init();
		}
		loct = localtime(&itime);
		loct->tm_mon++;	/* 1-12 for month */
		for (cp = list; *cp != EOS;) {
			flag = 0;
			cp = cmp(cp, loct->tm_min);
			cp = cmp(cp, loct->tm_hour);
			cp = cmp(cp, loct->tm_mday);
			cp = cmp(cp, loct->tm_mon);
			cp = cmp(cp, loct->tm_wday);
			if (flag == 0) {
				slp();
				ex(cp);
			}
			while (*cp++ != 0);
		}
	}
}
